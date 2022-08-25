# template.py
# Converts HTML template into C++ function that can be used on the ESP8622

from css_html_js_minify import html_minify
import sys
import re
import os

variables = []
file = "index.html"
function = None
width = 80

if len(sys.argv) == 1:
    print("Usage: template.py <input file> [width] [func name]")
    print("  Input File (required) - html file for processing")
    print("  Width      (optional) - max width of output block (chars)")
    print("  Func Name  (optional) - name of function generated")
    quit()

if len(sys.argv) > 1:
    file = sys.argv[1]

if len(sys.argv) > 2:
    width = int(sys.argv[2])

if len(sys.argv) > 3:
    function = sys.argv[3]
else:
    fname = os.path.basename(file).split(".")[0].capitalize()
    function = "make" + fname

with open(file) as f:
    raw = f.read()

no_vars = ""
for line in raw.split("\n"):
    tokens = re.split("\s+", line.strip())

    if len(tokens) == 4 and tokens[0] == "#define":
        variables.append(
            {
                "replace": "[%s]" % tokens[2],
                "type": tokens[1],
                "name": tokens[2],
                "format": tokens[3],
            }
        )
        continue

    no_vars += line + "\n"

minified = html_minify(no_vars, comments=False)

minified = minified.replace("<!-- -->", "")
minified = minified.replace('"', '\\"')
minified = minified.replace("%", "%%")
minified = minified.strip()

i = 0
replaced = ""
params = []

while i < len(minified):
    replaced += minified[i]

    for var in variables:
        if replaced.endswith(var["replace"]):
            replaced = replaced[: -len(var["replace"])]
            replaced += "%" + var["format"]
            params.append(var["name"])

    i += 1

funcParams = []
for var in variables:
    funcParams.append(var["type"] + " " + var["name"])

arguments = ", ".join(funcParams)

line = ""
stringLines = []
for char in replaced:
    line += char

    if len(line) >= width and not line.endswith("\\"):
        stringLines.append('"%s"' % line)
        line = ""

stringLines.append('"%s"' % line)
stringData = "\n            ".join(stringLines)

code = """void %s(char *buffer, %s) {
    sprintf(buffer,
            %s,
            %s);
}""" % (
    function,
    arguments,
    stringData,
    ", ".join(params),
)

print(code)
