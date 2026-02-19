"""
Node hierarchy matrix stack test with face tracking
"""

import basilisk as bsk
import glm
from math import sin, cos
import cv2
from collections import deque

# Set up face detection
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Warning: Could not open webcam", file=__import__('sys').stderr)

engine = bsk.Engine()
scene = bsk.Scene(engine, False, True, False)

camera = bsk.StaticCamera(engine)
scene.set_camera(camera)

camera.set_position((-20, 4, 4))

model = bsk.Mesh("models/john.obj")

node1 = bsk.Node(scene)
node2 = bsk.Node(node1, mesh=model, position=(0, 2, 3))
node3 = bsk.Node(node2, position=(0, 2, 3))

# Store past face positions and sizes for averaging
FACE_HISTORY_SIZE = 15  # Number of frames to average over
face_history = deque(maxlen=FACE_HISTORY_SIZE)  # Stores (x, y, size)

t = 0

try:
    while engine.is_running():
        engine.update()
        scene.update()

        # Capture and detect face
        if cap.isOpened():
            ret, frame = cap.read()
            if ret:
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                faces = face_cascade.detectMultiScale(
                    gray,
                    scaleFactor=1.1,
                    minNeighbors=5,
                    minSize=(30, 30)
                )
                
                if len(faces) > 0:
                    # Use the first detected face
                    x, y, w, h = faces[0]
                    # Use center of face for better tracking
                    face_x = x + w / 2  # Center x coordinate
                    face_y = y + h / 2  # Center y coordinate
                    # Use face size (area or average of width/height) as depth proxy
                    # Larger face = closer, smaller face = farther
                    face_size = (w + h) / 2  # Average of width and height
                    # Add to history (x, y, size)
                    face_history.append((face_x, face_y, face_size))
                else:
                    # If no face detected, keep the last known position in history
                    # (don't add anything, just use existing history)
                    pass
                
                # Average the face positions and size over the past n frames
                if len(face_history) > 0:
                    avg_x = sum(pos[0] for pos in face_history) / len(face_history)
                    avg_y = sum(pos[1] for pos in face_history) / len(face_history)
                    avg_size = sum(pos[2] for pos in face_history) / len(face_history)
                    # Convert face size to depth: larger size = closer (less negative z)
                    # Adjust these scaling factors as needed
                    base_z = -20
                    depth_offset = (avg_size - 100) / 20  # Adjust based on typical face size

                    camera_z = depth_offset -20
                    camera_x = -avg_x / 40 + 8
                    camera_y = -avg_y / 40 + 10
                    # Set camera position based on averaged face position and depth
                    camera.set_position((camera_z, camera_y, camera_x))


        scene.render()
        engine.render()

finally:
    # Clean up
    if cap.isOpened():
        cap.release()
    cv2.destroyAllWindows()