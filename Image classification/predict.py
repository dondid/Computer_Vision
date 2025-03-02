import os
import sys
import warnings
warnings.filterwarnings('ignore')
os.environ['ENABLE_ONEDNN_OPTS'] = '0'
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

from tensorflow.keras.models import load_model
from tensorflow.keras.preprocessing import image
import numpy as np

model = load_model("C:/openCV/project/T01/dog_emotion_model.h5", compile=False)
img = image.load_img(sys.argv[1], target_size=(150, 150))
img_array = image.img_to_array(img) / 255.0
img_array = np.expand_dims(img_array, axis=0)

predictions = model.predict(img_array, verbose=0)[0]
result_lines = [
    f"Angry: {predictions[0]*100:.2f}%",
    f"Happy: {predictions[1]*100:.2f}%",
    f"Relaxed: {predictions[2]*100:.2f}%",
    f"Sad: {predictions[3]*100:.2f}%",
    "",
    f"Predominant state: {['Angry', 'Happy', 'Relaxed', 'Sad'][np.argmax(predictions)]} ({np.max(predictions)*100:.2f}%)"
]

print('\n'.join(result_lines))
