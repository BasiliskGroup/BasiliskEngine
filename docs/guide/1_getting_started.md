# Getting Started
This page will cover Basilisk installation and the basic boilerplate code.

## Installation 
To start, you will need to install Basilisk engine. To do so, simply run the following command from the terminal:

```cmd
pip install basilisk-engine
```

Now you will be able to import the package with `import basilisk`. Since Basilisk is fully open source, you also have the option to download the code from the [github](https://github.com/Loffelt/BasiliskEngine) if you prefer.

## First Program
Every Basilisk prorgam has an [Engine]() that handles the high level functionality of Basilisk. Additionally, you will want a [Scene]() which will hold your applications objects.

```py
# Import basilisk into the project. We use bsk as convention
import basilisk as bsk 

# Initialize objects for the engine and the scene
engine = bsk.Engine()
scene = bsk.Scene()
```

Here we will introduce an important paradigm of Basilisk's design that will reoccur throughout this guide. Most things you will use in Basilisk are objects, such as the engine and the scene. You can use these objects wherever and whenever you want, but you have to tell the engine that you are using them. So, the next thing we will do is tell the engine that we want to use the scene we just created.

```py
# Let the engine know we are using this scene right now
engine.scene = scene
```

Though this extra line may seem tedious, it allows for a great deal of flexibility, as you can mix Basilisk objects around in any configuration you would like, treating Basilisk like a state machine.

Now, we will set up the game loop. We use a while loop by convention.

```py
while engine.running: # Check that the engine is still running
    engine.update()   # Update and render the engine
```

The `engine.running` attribute is just a boolean flag that tells the user if the engine is running still and has not been stoped for any reason. The `engine.update()` function will tell the engine to handle all rendering, physics, and inputs for the current tick.

With just these six lines of code, you can now run the python file, and you should see something like this. Note that you can free your mouse by pressing escape (this behavior can be changed if desired see input section in [Engine]() reference page):

<div align="center">
    <img src="../images/0_boilerplate.png" alt="mud" width="400"/>
</div>

Congratulations you have just finished your first basilisk program. In the later sections of the guide, we will learn how to add more objects to the scene to make it a bit more intresting.

## Full Code
For clearity, here is the full code used in this tutorial.

```py
import basilisk as bsk

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

while engine.running:
    engine.update()
```