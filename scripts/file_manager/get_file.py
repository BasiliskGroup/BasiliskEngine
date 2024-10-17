from tkinter import Tk     # from tkinter import Tk for Python 3.x
from tkinter.filedialog import askopenfilename, asksaveasfile

def save_file_selector():
    Tk().withdraw() 
    file = asksaveasfile(initialfile = "Untitled.gltf", filetypes=[("GLTF files","*.gltf")], initialdir="saves")
    if not file: return None
    if not file.name.endswith('.gltf'): return file.name + ".gltf"
    return file.name

def load_file_selector():
    Tk().withdraw() 
    file = askopenfilename(filetypes=[("GLTF files","*.gltf")], initialdir="saves")
    if not file: return None
    return file