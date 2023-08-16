import json

# Fill in the correct path here
with open("presetcategories.json") as f:
    data = json.load(f)

indent = 0


def printi(*values: object, **kwargs) -> None:
    print(" " * 4 * indent + values[0], *values[1:], **kwargs)


def handleItem(item):
    global indent
    if "items" in item:
        printi("MsBasicItem {")
        indent += 1
        printi('/*title=*/ u"{}",'.format(item["name"]))
        printi("/*subItems=*/ {")
        indent += 1
        for subitem in item["items"]:
            handleItem(subitem)
        indent -= 1
        printi("}")
        indent -= 1
        printi("},")
        print()
    else:
        printi(
            "MsBasicItem {{ midi::Program({0:<3} {1}) }},".format(
                str(item["bank"]) + ",", item["program"]
            )
        )


for item in data:
    handleItem(item)
