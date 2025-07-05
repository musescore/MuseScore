from lxml import etree as ET
import argparse
import copy

def extract_fret_harmony_pairs(mscx_path):
    tree = ET.parse(mscx_path)
    root = tree.getroot()

    elements = []

    for parent in root.iter():
        children = list(parent)
        for i in range(len(children) - 1):
            first = children[i]
            second = children[i + 1]

            if (first.tag == "FretDiagram" and second.tag == "Harmony") or \
               (first.tag == "Harmony" and second.tag == "FretDiagram"):
                elements.append(first)
                elements.append(second)

    pairs = []
    i = 0
    while i < len(elements) - 1:
        if elements[i].tag == "FretDiagram" and elements[i + 1].tag == "Harmony":
            pairs.append((elements[i], elements[i + 1]))
            i += 2
        elif elements[i].tag == "Harmony" and elements[i + 1].tag == "FretDiagram":
            pairs.append((elements[i + 1], elements[i]))
            i += 2
        else:
            i += 1

    return pairs

def remove_barre_overlapping_dots(fret_elem, offset):
    strings = fret_elem.findall("string")

    for barre in fret_elem.findall("barre"):
        start = int(barre.get("start", -1))
        end = int(barre.get("end", -1))
        fret = int(barre.text.strip()) + offset

        for string in strings:
            string_no = int(string.get("no", -1))
            dot = string.find("dot")
            if dot is not None and start <= string_no <= end:
                dot_fret = int(dot.get("fret", -1)) + offset
                if dot_fret == fret:
                    string.remove(dot)

def extract_pattern_from_fretdiagram(fret_elem):
    strings_count = 6
    offset_elem = fret_elem.getparent().find("fretOffset")
    offset = int(offset_elem.text) if offset_elem is not None else 0

    pattern = ['-' for _ in range(strings_count)]
    barre_parts = []

    for string_elem in fret_elem.findall("string"):
        string_no = int(string_elem.get("no", -1))
        if not (0 <= string_no < strings_count):
            continue

        marker = string_elem.find("marker")
        dot_elems = string_elem.findall("dot")
        dot_descriptions = []

        if marker is not None and not dot_elems:
            marker_type = marker.text.strip().lower()
            if marker_type == "cross":
                pattern[string_no] = "X"
            elif marker_type == "circle" or marker_type == "o":
                pattern[string_no] = "O"

        elif dot_elems:
            for dot_elem in dot_elems:
                fret = int(dot_elem.get("fret", -1))
                dot_type = dot_elem.text.strip().lower() if dot_elem.text else "normal"

                type_char = "O"
                if dot_type == "cross":
                    type_char = "X"
                elif dot_type == "square":
                    type_char = "S"
                elif dot_type == "triangle":
                    type_char = "T"

                if fret >= 0:
                    dot_descriptions.append("%d-%s" % (fret + offset, type_char))

            if dot_descriptions:
                pattern[string_no] = "[" + ",".join(dot_descriptions) + "]"

    for barre_elem in fret_elem.findall("barre"):
        start = int(barre_elem.get("start", -1))
        end = int(barre_elem.get("end", -1))
        barre_fret_text = barre_elem.text.strip()

        if not barre_fret_text.isdigit() or not (0 <= start < strings_count):
            continue

        if end == -1:
            end = strings_count - 1
        if end < start or end >= strings_count:
            continue

        barre_fret = int(barre_fret_text) + offset

        for i in range(start, end + 1):
            current = pattern[i]
            if current.startswith("[") and current.endswith("]"):
                inner = current[1:-1]
                new_inner = []
                for dot_str in inner.split(","):
                    parts = dot_str.strip().split("-")
                    if len(parts) != 2:
                        continue
                    try:
                        fret_val = int(parts[0])
                    except ValueError:
                        continue
                    if fret_val != barre_fret:
                        new_inner.append(dot_str)
                if new_inner:
                    pattern[i] = "[" + ",".join(new_inner) + "]"
                else:
                    pattern[i] = "-"
            elif current == str(barre_fret):
                pattern[i] = "-"

        barre_parts.append("B%d[%d-%d]" % (barre_fret, start, end))

    base_pattern = ''.join(pattern)
    if barre_parts:
        base_pattern += ';' + ';'.join(barre_parts)

    return base_pattern

def get_harmony_name(harmony_elem):
    name_elem = harmony_elem.find("name")
    return name_elem.text.strip().lower() if name_elem is not None and name_elem.text else "unknown"

def build_output_xml(pairs):
    data_elem = ET.Element("Data")

    for fret_elem, harmony_elem in pairs:
        name = get_harmony_name(harmony_elem)

        htd = ET.SubElement(data_elem, "HarmonyToDiagram")
        fret_diagram_elem = ET.SubElement(htd, "FretDiagram")

        for child in fret_elem:
            if child.tag == "eid":
                continue
            elif child.tag == "fretDiagram":
                fret = copy.deepcopy(child)

                offset_elem = fret_elem.getparent().find("fretOffset")
                offset = int(offset_elem.text) if offset_elem is not None else 0
                remove_barre_overlapping_dots(fret, offset)

                fret_diagram_elem.append(fret)

                pattern = extract_pattern_from_fretdiagram(fret)
                pattern_elem = ET.SubElement(htd, "pattern")
                pattern_elem.text = pattern

            elif child.tag == "string" and fret_elem.find("fretDiagram") is not None and child not in fret_elem.find("fretDiagram"):
                continue
            else:
                fret_diagram_elem.append(copy.deepcopy(child))

        harmony_out = ET.SubElement(htd, "Harmony")
        name_elem = ET.SubElement(harmony_out, "name")
        name_elem.text = name

    return data_elem

def indent(elem, level=0):
    i = "\n" + level * "    "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "    "
        for child in elem:
            indent(child, level + 1)
        if not elem[-1].tail or not elem[-1].tail.strip():
            elem[-1].tail = i
    if level and (not elem.tail or not elem.tail.strip()):
        elem.tail = i
    return elem

def main():
    # Example: python2 parse_mscx.py --in "/path/to/file.mscx" --out /path/to/result.xml
    parser = argparse.ArgumentParser(description="Extract Harmony and FretDiagram pairs from .mscx and output XML.")
    parser.add_argument('--in', dest='input_file', required=True, help='Path to the input .mscx file')
    parser.add_argument('--out', dest='output_file', required=True, help='Path to the output .xml file')
    args = parser.parse_args()

    pairs = extract_fret_harmony_pairs(args.input_file)
    root_elem = build_output_xml(pairs)
    indent(root_elem)

    tree = ET.ElementTree(root_elem)
    tree.write(args.output_file, encoding="utf-8", xml_declaration=True)

    print("Finished")

if __name__ == "__main__":
    main()
