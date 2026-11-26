"""Lit les déclarations C++ d'EcoSim pour alimenter les diagrammes détaillés.

Ce n'est pas un analyseur C++ complet : il couvre les écritures utilisées dans ce projet.
"""
import io
import os
import re
from dataclasses import dataclass, field


@dataclass
class Member:
    """Un membre ou une déclaration libre."""
    kind: str                 # "attribute" ou "function"
    name: str
    text: str                 # écriture UML, sans la visibilité
    visibility: str = "+"
    static: bool = False
    abstract: bool = False


@dataclass
class Type:
    """Une classe, une structure, une énumération ou un alias."""
    kind: str                 # "class", "struct", "enum" ou "alias"
    name: str
    module: str
    path: str
    bases: str = ""
    template: str = ""
    local: bool = False       # vrai pour un type propre à un fichier .cpp
    members: list = field(default_factory=list)
    values: list = field(default_factory=list)


@dataclass
class Module:
    """Un module : ses imports et ses déclarations hors classes."""
    name: str
    path: str
    imports: list = field(default_factory=list)
    functions: list = field(default_factory=list)
    constants: list = field(default_factory=list)


def clean(text):
    """Retire commentaires et directives, sans toucher aux chaînes."""
    pattern = re.compile(r'("(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])+\')|//[^\n]*|/\*.*?\*/', re.S)
    text = pattern.sub(lambda m: m.group(1) or " ", text.replace("\r\n", "\n"))
    return re.sub(r"^[ \t]*#[^\n]*", "", text, flags=re.M)


def closing(text, start):
    """Position du signe qui ferme celui placé en start."""
    pairs = {"{": "}", "(": ")", "<": ">", "[": "]"}
    opening, depth, i = text[start], 0, start
    while i < len(text):
        char = text[i]
        if char in "\"'":
            i += 1
            while text[i] != char:
                i += 2 if text[i] == "\\" else 1
        elif char == opening:
            depth += 1
        elif char == pairs[opening]:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("bloc non fermé : " + text[start:start + 60])


def split_top(text, separator=","):
    """Découpe aux séparateurs situés hors de toute parenthèse."""
    parts, depth, current = [], 0, ""
    for char in text:
        if char in "({<[":
            depth += 1
        elif char in ")}>]":
            depth -= 1
        if char == separator and depth == 0:
            parts.append(current)
            current = ""
        else:
            current += char
    parts.append(current)
    return [p.strip() for p in parts if p.strip()]


def squeeze(text):
    """Une seule espace entre les mots, et sans le préfixe std:: ou ecosim::."""
    return " ".join(text.replace("std::", "").replace("ecosim::", "").split())


def outer_paren(header):
    """Position de la parenthèse des paramètres, ou -1 pour une variable."""
    operator = re.search(r"\boperator\s*(\(\)|\[\]|[=!<>+\-*/]+)", header)
    if operator:
        return header.find("(", operator.end())
    angle = 0
    for i, char in enumerate(header):
        if char == "<":
            angle += 1
        elif char == ">":
            angle -= 1
        elif char in "={" and angle == 0:
            return -1
        elif char == "(" and angle == 0:
            return i
    return -1


def parameter(text):
    """« const T& nom = valeur » devient « nom : const T& »."""
    text = squeeze(split_top(text, "=")[0])
    match = re.match(r"^(.*?[\s&*>])([A-Za-z_]\w*)$", text)
    if match and match.group(1).strip() not in ("const", "unsigned"):
        return f"{match.group(2)} : {match.group(1).strip()}"
    return text


def function(header, visibility, class_name):
    """Construit la description d'une fonction à partir de son en-tête."""
    header = squeeze(header)
    start = outer_paren(header)
    end = closing(header, start)
    before, inside, after = header[:start].strip(), header[start + 1:end], header[end + 1:]
    words = [w for w in before.split() if w != "[[nodiscard]]"]
    static = "static" in words
    virtual = "virtual" in words
    words = [w for w in words if w not in ("static", "virtual", "explicit", "inline", "constexpr", "friend")]
    name = words.pop()
    if words and words[-1] == "operator":
        name = words.pop() + name
    returned = " ".join(words)
    text = f"{name}({', '.join(parameter(p) for p in split_top(inside))})"
    if returned:
        text += f" : {returned}"
    if re.search(r"\bconst\b", after.split("->")[0]):
        text += " {const}"
    if re.search(r"=\s*delete", after):
        text += " {supprimé}"
    elif re.search(r"=\s*default", after):
        text += " {par défaut}"
    abstract = virtual and bool(re.search(r"=\s*0\b", after))
    return Member("function", name, text, visibility, static, abstract)


def attribute(statement, visibility):
    """Construit la description d'un attribut ou d'une constante."""
    statement = squeeze(statement)
    static = bool(re.match(r"^(inline )?static\b", statement))
    constant = "constexpr" in statement.split("{")[0].split()
    statement = re.sub(r"\b(static|inline|constexpr|mutable)\b ?", "", statement)
    pointer = re.match(r"^(.*?)\(\s*\*\s*(\w+)\s*\)\s*\((.*)\)$", statement)
    if pointer:
        arguments = ", ".join(parameter(p) for p in split_top(pointer.group(3)))
        name = pointer.group(2)
        return Member("attribute", name, f"{name} : {pointer.group(1).strip()} (*)({arguments})", visibility)
    angle, cut = 0, len(statement)
    for i, char in enumerate(statement):
        angle += {"<": 1, ">": -1}.get(char, 0)
        if char in "{=" and angle == 0:
            cut = i
            break
    declaration, value = statement[:cut].strip(), statement[cut:].strip()
    kind, name = declaration.rsplit(" ", 1)
    text = f"{name} : {kind}"
    if constant:
        value = value[1:].strip() if value.startswith("=") else value
        if value.startswith("{") and value.endswith("}"):
            value = value[1:-1]
        text += f" = {value} {{constante}}"
    return Member("attribute", name, text, visibility, static)


def statements(body):
    """Parcourt un bloc et rend des couples (en-tête, contenu entre accolades ou None)."""
    i, start = 0, 0
    while i < len(body):
        char = body[i]
        if char in "\"'":
            i += 1
            while body[i] != char:
                i += 2 if body[i] == "\\" else 1
        elif char in "([":
            i = closing(body, i)
        elif char == ";":
            if body[start:i].strip():
                yield body[start:i].strip(), None
            start = i + 1
        elif char == "{":
            header = body[start:i].strip()
            bare = re.sub(r"^((public|private|protected)\s*:\s*|export\s+|template\s*<[^>]*>\s*)+", "", header)
            end = closing(body, i)
            if re.match(r"(namespace|class|struct|enum)\b", bare) or header == "export":
                yield header, body[i + 1:end]
                i = end
                start = i + 1
            elif outer_paren(bare) >= 0:
                parameters_end = closing(bare, outer_paren(bare))
                if re.search(r"(?<!:):(?!:)", bare[parameters_end:]):
                    # Liste d'initialisation : le corps suit le dernier élément initialisé.
                    while True:
                        after = end + 1
                        while body[after].isspace():
                            after += 1
                        if body[after] == ",":
                            i = body.index(",", after)
                            while body[i] not in "{(":
                                i += 1
                            end = closing(body, i)
                        else:
                            end = closing(body, after) if body[after] == "{" else end
                            break
                yield header, body[i + 1:end]
                i = end
                start = i + 1
                while start < len(body) and body[start] in " \n\t;":
                    start += 1
                i = start - 1
            else:
                i = end
        i += 1


def read_scope(body, module, types, owner=None, access="+", local=False):
    """Lit un bloc : espace de noms, ou corps d'une classe si owner est donné."""
    for header, content in statements(body):
        while True:
            label = re.match(r"^(public|private|protected)\s*:(?!:)\s*", header)
            if not label:
                break
            access = {"public": "+", "private": "-", "protected": "#"}[label.group(1)]
            header = header[label.end():]
        header = re.sub(r"^export\b\s*", "", header)
        template = ""
        found = re.match(r"^template\s*<([^>]*)>\s*", header)
        if found:
            template, header = found.group(1).strip(), header[found.end():]
        if content is not None and (header.startswith("namespace") or header == ""):
            anonymous = header.strip() == "namespace"
            read_scope(content, module, types, local=local or anonymous)
            continue
        kind = re.match(r"^(enum class|enum|class|struct)\s+(\w+)(.*)$", header, re.S)
        if kind and outer_paren(header) < 0:
            if content is None:
                continue   # simple déclaration anticipée
            word, name, rest = kind.groups()
            full = f"{owner.name}::{name}" if owner else name
            item = Type("enum" if word.startswith("enum") else word, full, module.name, module.path,
                        squeeze(rest.replace("final", "")).lstrip(": ").replace("public ", ""),
                        template, local)
            types.append(item)
            if item.kind == "enum":
                item.values = [squeeze(v) for v in split_top(content)]
            else:
                read_scope(content, module, types, item, "-" if word == "class" else "+", local)
            continue
        if re.match(r"^(module|import|static_assert)\b", header):
            continue
        alias = re.match(r"^using\s+(\w+)\s*=\s*(.*)$", header, re.S)
        if alias:
            if owner is None:
                types.append(Type("alias", alias.group(1), module.name, module.path,
                                  squeeze(alias.group(2)), local=local))
            continue
        if header.startswith("using ") or not header:
            continue
        if outer_paren(header) >= 0 and not re.search(r"\(\s*\*\s*\w+\s*\)", header):
            member = function(header, access, owner.name if owner else "")
            (owner.members if owner else module.functions).append(member)
        else:
            full = header if content is None else header
            member = attribute(full, access)
            (owner.members if owner else module.constants).append(member)


def read_project(root):
    """Lit tout le dossier src : rend les modules et les types."""
    modules, types = {}, []
    sources = sorted(os.path.join(d, f).replace("\\", "/")
                     for d, _, files in os.walk(os.path.join(root, "src"))
                     for f in files if f.endswith((".ixx", ".cpp", ".hpp")))
    for path in sorted(sources, key=lambda p: not p.endswith(".ixx")):
        text = clean(io.open(path, encoding="utf-8").read())
        if path.endswith(".hpp"):
            name = os.path.basename(path)
            module = modules.setdefault(name, Module(name, os.path.relpath(path, root).replace(os.sep, "/")))
            read_scope(text, module, types)
            continue
        declared = re.search(r"^\s*(?:export\s+)?module\s+([\w.]+)\s*;", text, re.M)
        if not declared:
            continue       # main.cpp
        name = declared.group(1)
        relative = os.path.relpath(path, root).replace("\\", "/")
        module = modules.setdefault(name, Module(name, relative))
        for imported in re.findall(r"^\s*(?:export\s+)?import\s+(ecosim\.[\w.]+)\s*;", text, re.M):
            if imported not in module.imports:
                module.imports.append(imported)
        body = text[declared.end():]
        if path.endswith(".ixx"):
            read_scope(body, module, types)
        else:
            # Dans un .cpp, seuls les types propres au fichier sont nouveaux.
            for found in re.finditer(r"^[ \t]*(class|struct|enum class)\s+\w+[^;{()]*\{", body, re.M):
                end = closing(body, found.end() - 1)
                holder = Module(name, relative)
                read_scope(body[found.start():end + 1] + ";", holder, types, local=True)
    return modules, types
