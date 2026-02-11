import os

script_dir = os.path.dirname(os.path.abspath(__file__))
sibling_dir = os.path.join(os.path.dirname(script_dir), 'saved_state')
for filename in os.listdir(sibling_dir):
    path = os.path.join(sibling_dir, filename)

    if filename.endswith(".txt") and os.path.isfile(path):
        with open(path, "w") as f:
            pass
        print(f"Cleared: {filename}")
