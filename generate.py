import os
import sys

sys.path.insert(0, 'plugin/libs/godot-cpp')
import binding_generator

## Location of our extension_api.json file
## json_api_file = os.path.join(os.getcwd(), 'plugin/libs/godot-cpp/gdextension', 'extension_api.json')

## override for now, we use a custom one until upstream PR is merged
json_api_file = os.path.join(os.getcwd(), 'plugin/libs', 'extension_api.json')
binding_generator.generate_bindings(json_api_file, True, '64', 'single', 'plugin/libs/godot-cpp')
