# Basilisk Engine
![image](https://github.com/user-attachments/assets/5e39445c-e0da-452c-9f18-e590cca948c4)
Basilisk is a 3D engine package for python that can create visualizations, simulations, and video games from the comfort and ease of Python. Basilisk is designed for quick and effortless development with a powerful backend engine that supports larger scaled projects. The engine automatically handles all graphics and physics for you, with the option to inject your own functionality if desired.

<p align="center">
    <img src="images/mud.png" alt="mud" width="400"/>
    <img src="images/foil.png" alt="foil" width="400"/>
    <img src="images/cloth.png" alt="mud" width="400"/>
    <img src="images/floor.png" alt="mud" width="400"/>
</p>

# Quick Start
## Installation 
To start, you will need to install Basilisk engine. To do so, simply run the following command from the terminal:

```cmd
pip install basilisk-engine
```

Now you will be able to import the package with `import basilisk`. Since Basilisk is fully open source, you also have the option to download the code from the [github](https://github.com/Loffelt/BasiliskEngine) if you prefer.

## First Program
Every Basilisk prorgam has an [Engine]() that handles the high level functionality of Basilisk. Additionally, you will want a [Scene]() which will hold the application's objects.

```py
# Import basilisk into the project. We use bsk as convention
import basilisk as bsk 

# Initialize objects for the engine and the scene
engine = bsk.Engine()
scene = bsk.Scene(engine)
```

Here we will introduce an important paradigm of Basilisk's design that will reoccur throughout this guide. Most things used in Basilisk are objects, such as the engine and the scene. You can use these objects wherever and whenever you want. In this instance, we pass the engine to the scene, so that Basilisk knows the scene is part of the engine. Now, we will set up the game loop. We use a while loop by convention.

```py
while engine.running: # Check that the engine is still running
    scene.update()    # Update the scene and render to temporary frame
    engine.update()   # Update the engine and render to the screen 
```

The `engine.running` attribute is just a boolean flag that tells the user if the engine is running still and has not been stoped for any reason. The `scene.update()` function will render the scene and handle physics/object updates for the tick. The `engine.update()` function will tell the engine to handle all frame rendering and inputs.

With just these six lines of code, you can now run the python file, and you should see something like this. Note that you can free your mouse by pressing escape (this behavior can be changed if desired see input section in [Engine]() reference page):

<div align="center">
    <img src="docs/images/0_boilerplate.png" alt="mud" width="400"/>
</div>

Congratulations you have finished your first Basilisk program!

## Full Code
For clarity, here is the full code used in this tutorial.

```py
import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene(engine)

while engine.running:
    scene.update()
    engine.update()
```

## Learn more at the [Basilisk Engine Website](https://basilisk-website.vercel.app)
Webpage still under development.
