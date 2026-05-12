/usr/src/tensorrt/bin/trtexec \
--onnx=yolov26m_best.onnx \
--saveEngine=pcb_inspection_model_v2.engine \
--fp16 \

--minShapes=images:1x3x640x640 \
--optShapes=images:1x3x640x640 \
--maxShapes=images:1x3x640x640
