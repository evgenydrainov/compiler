# Regenerates the Node visualizers in compiler.natvis from the node structs in parser.h.
# Run this after adding a node kind or changing a node struct's fields:
#     python src/gen_natvis.py

import os
import re

SRC_DIR = os.path.dirname(os.path.abspath(__file__))
PARSER_H = os.path.join(SRC_DIR, "parser.h")
NATVIS = os.path.join(SRC_DIR, "compiler.natvis")

BEGIN_MARKER = "<!-- BEGIN GENERATED NODE VISUALIZERS (src/gen_natvis.py) -->"
END_MARKER = "<!-- END GENERATED NODE VISUALIZERS -->"

def parse_node_structs(text):
	structs = []
	for m in re.finditer(r"struct (\w+Node) : public Node\s*\{(.*?)\n\};", text, re.S):
		name, body = m.group(1), m.group(2)
		kind = None
		fields = []
		for line in body.splitlines():
			line = line.split("//")[0].strip()
			if not line:
				continue
			k = re.match(r"static constexpr NodeKind KIND = NodeKind_(\w+);", line)
			if k:
				kind = k.group(1)
				continue
			f = re.search(r"(\w+)\s*;$", line)
			if f:
				fields.append(f.group(1))
		if kind is None:
			raise SystemExit(f"{name} has no KIND")
		structs.append((name, kind, fields))
	return structs

def generate(structs):
	out = []
	out.append("  <!-- Node shows the fields of its derived type based on kind.")
	out.append("       Inheritable=\"false\" so the derived types don't pick this up and recurse. -->")
	out.append("  <Type Name=\"Node\" Inheritable=\"false\">")
	out.append("    <DisplayString>{kind,en}</DisplayString>")
	out.append("")
	out.append("    <Expand>")
	for name, kind, _ in structs:
		out.append(f"      <ExpandedItem Condition=\"kind == NodeKind_{kind}\">*({name} *)this</ExpandedItem>")
	out.append("    </Expand>")
	out.append("  </Type>")

	# Derived types list their own fields and fold the common Node fields into
	# one raw row, instead of the default base-class row that would recurse.
	for name, _, fields in structs:
		out.append("")
		out.append(f"  <Type Name=\"{name}\">")
		out.append("    <DisplayString>{kind,en}</DisplayString>")
		out.append("")
		out.append("    <Expand>")
		for field in fields:
			out.append(f"      <Item Name=\"{field}\">{field}</Item>")
		out.append("      <Item Name=\"[Node]\">*(Node *)this,!</Item>")
		out.append("    </Expand>")
		out.append("  </Type>")
	return "\n".join(out)

def main():
	with open(PARSER_H, encoding="utf-8") as f:
		structs = parse_node_structs(f.read())

	with open(NATVIS, encoding="utf-8", newline="") as f:
		natvis = f.read()

	begin = natvis.index(BEGIN_MARKER) + len(BEGIN_MARKER)
	end = natvis.index(END_MARKER)
	natvis = natvis[:begin] + "\n" + generate(structs) + "\n\n  " + natvis[end:]

	with open(NATVIS, "w", encoding="utf-8", newline="\n") as f:
		f.write(natvis)

	print(f"generated visualizers for {len(structs)} node types")

if __name__ == "__main__":
	main()
