class Line:
    def __init__(self, line_type, identifier, attribs) -> None:
        self.line_type = line_type
        self.identifier = identifier
        self.attribs = attribs

    def __repr__(self) -> str:
        return f"<{self.line_type} : {self.identifier}>"

def load_scene(scene, file: str):
    """
    Loads a scene file onto an existing scene, if given, or a new scene which is returned.
    """

    lines = get_lines(file)
    if not lines[0].startswith("bsk"): raise RuntimeError('Cannot load file ' + file + ' because it is not of type "bsk"')

    scene_decriptor = convert_line(lines[1])

    if scene_decriptor.identifier != "bsk_scene": raise RuntimeError("Scene file " + file + " must have a scene descriptor")

    current_item = None
    for line in lines[2:]:
        line = convert_line(line)

        if line.line_type == "declaration":
            current_item = add_delcaration(scene, line)

        else:
            apply_modifier(scene, line, current_item)


def add_delcaration(scene, line):
    current_object = None

    match line.identifier:
        case "material":
            current_object = create_material(scene, line)
        case "node":
            current_object = create_node(scene, line)

    return current_object


def apply_modifier(scene, line, current_object):
    value = line.attribs[line.identifier]
    match line.identifier:
        case "position":
            current_object.position = value
        case "rotation":
            current_object.rotation = value
        case "scale":
            current_object.scale = value
        case "mesh":
            current_object.model = scene.node_handler.scene.model_handler.add(vbo=value)
        case "material":
            current_object.model.material = value


def create_material(scene, line):
    scene.material_handler.add(**line.attribs)
    return None


def create_node(scene, line):
    node = scene.node_handler.add(name=line.attribs["name"])

    return node


def get_lines(file: str) -> list[str]:
    """
    Returns a list of strings of all the lines in the given file.
    Automatically removes comments and empty lines.
    """
    
    with open(file, 'r') as file:
        lines = list(file)

    line_index = 0
    while line_index < len(lines):
        lines[line_index] = lines[line_index].strip()

        # Get rid of empty lines
        if len(lines[line_index]) < 1:
            lines.pop(line_index)
            continue
        # Get rid of comment lines
        if lines[line_index][0] == '#':
            lines.pop(line_index)
            continue


        line_index += 1

    return lines

def convert_line(line):
    """
    Converts a line string into a Line class instance with information seperated out. 
    """

    # Modifier Line
    if not line.startswith('['):
        token = line

        attrib, value = token.split('=')
        attrib = attrib.strip()
        value = convert_value(value.strip())

        return Line("modifier", attrib, {attrib : value})
        
    # Declaration Line
    tokens = get_tokens(line)
    line = line[1:-1]
    attrib_values = {}

    for token in tokens:
        if "=" in token:
            attrib, value = token.split('=')
            attrib_values[attrib.strip()] = convert_value(value.strip())
        else:
            identifier = token

    return Line("declaration", identifier, attrib_values)


def get_tokens(line: str) -> list[str]:
    """
    Returns the tokens of a line
    """
    
    # A list of all the tokens
    tokens = []

    if not line.startswith('['):
        return line

    line = line[1:-1]

    # State variables
    in_quotes = False
    token = ""

    # Loop through the line
    for i in range(len(line)):
        char = line[i]

        # Add a token at a space if it isnt in a quote
        if char == " " and not in_quotes:
            tokens.append(token)
            token = ""
            i += 1
        
        # Flip the in_quotes bit if char is a double quote
        elif char == '"' or char == '(' or char == ')':
            in_quotes = not in_quotes

        # Increment 
        token = token + char
        i += 1
    
    # Add the token found at the end
    tokens.append(token)

    return tokens


def convert_value(value: str):
    """
    Converts a value string into the correct data type
    """

    # String
    if value.startswith('"'):
        return value[1:-1]
    
    # Tuple
    elif value.startswith('('):
        values = []
        [values.append(float(tuple_value.strip())) for tuple_value in value[1:-1].split(',')]
        return tuple(values)
    
    # Float
    return float(value)