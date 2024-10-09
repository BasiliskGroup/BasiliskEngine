# Basilisk Scene File Specifications

## Header
Files should start with `bsk` followed by the version number of basilisk when the file was saved:

```
bsk 0.0.1
```

## Comments
Marked with a `#`

```
# This is a comment
```
Additionally, lines of whitespace are ignored

## Files

It is expected that all models and texture files will be provided. Right now, they are currently stored on the top level folders named `models` and `textures`, though this may change to be with the save in later versions.

## Properties and Data Types
Properties are given by `<property_name>=<value>`. String values are enclosed by double quotes. Numeric values are assumed to be floats (never integers). Tuple values are enclosed by parentheses, and are assumed to contain only floats.


## Scene Descriptor
Give information about the scene.

```
[bsk_scene name="Sample Scene" version=1]
```

First, enlcose the scene decriptor in brackets. Then use the `bsk_scene` keyword to indicated that the line is a scene descriptor. The scene descriptor takes properties name and version (which differs from the basilisk version).

## Materials
Define the use of colors and textures on nodes with meshes. The base material will generally always be included, so we will use it as an example:

```
[material name="Base" color=(.8, .8, .8)]
```

The keyword for declaring a material is `material`. All materials should be given a unique name. Materials may have the following properties:

- `color=(.8, .8, .8)`
- `specular=1`
- `specular_exponent=64`
- `alpha=1`
- `texture="None"` or `texture="texutre_name"`
- `normal_map="None"` or `normal_map="normal_texture_name"`

## Nodes
### Declaration

Nodes encapuslate all game objects. They often will have modifiers on them to add the correct behavior. 

```
[node name="cube" id="cube_1" parent="."]
```

The keyword for adding a node is `node`. When declaring a node, a name, id, and parent should be given. If no parent is given, it will default to the root (`"."`). Parent nodes should be identified by their unique id, not their name.

### Modifiers
Modifiers are like kwargs for nodes. They are not nessecary, but can add behavior to a node. Modifiers should be on the lines proceeding a node declaration. Modifiers are not enclosed in brackets. The general modifier format is:
```
<modifier_name> = <value>
```

#### `position = (x, y, z)`
Default value is `(0, 0, 0)`

#### `rotation = (x, y, z)`
Given in radian euler angles. The default value is `(0, 0, 0)`

#### `scale = (x, y, z)`
Default value is `(1, 1, 1)`

#### `mesh = "mesh_name"`
Default value is `"Cube"`

#### `material = "Material_Name"`
Default value is `"Base"`

#### `collider = ???`
Default value is "None"

#### `camera = "Free"`
Default value is `"None"`

## Skeletons
...

## Example Scene
```
bsk 0.0.1

# Scene Descriptor
[bsk_scene name="Sample Scene" version=1]

# Materials
[material name="Base" color=(.8, .8, .8)]
[material name="Brick" texture="brick"]

# Nodes
[node name="camera" id="cube_1" parent="."]
camera = "Free"

[node name="cube" id="cube_1" parent="."]
mesh = "Cube"
material = "Base"
position = (1, 1, 1)
rotation = (3.14, 0, 0)
physics_body = "True"
collider = ???

[node name="ball" id="ball_1" parent="cube_1"]
mesh = "Sphere"
material = "Brick"
position = (1, 2, 1)
scale = (.25, .5, .25)
```