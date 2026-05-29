import re

with open('camera_test/camera_test.ino', 'r') as f:
    content = f.read()

# Handle timeout branch
content = content.replace('} else {\n    resetAllButtons();showPhotoView(photoViewIndex);return;\n  }', '} else {\n    neoOff();\n    resetAllButtons();showPhotoView(photoViewIndex);return;\n  }')

# Handle BTN_BOOT confirm branch
content = content.replace('if(evt.pin==BTN_BOOT) photoViewDeleteCurrent();', 'if(evt.pin==BTN_BOOT) { neoOff(); photoViewDeleteCurrent(); }')

# Handle cancel branch
content = content.replace('else{resetAllButtons();showPhotoView(photoViewIndex);}', 'else { neoOff(); resetAllButtons(); showPhotoView(photoViewIndex); }')

with open('camera_test/camera_test.ino', 'w') as f:
    f.write(content)
