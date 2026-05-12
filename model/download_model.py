from roboflow import Roboflow
rf = Roboflow(api_key="0KeBuMMCJy2G8QXyR1EI")
project = rf.workspace("university-2xdiy").project("pcb-defects-chi1b")
model = project.version(2).model

# Downloads best.onnx to your folder
model.download("pt")
