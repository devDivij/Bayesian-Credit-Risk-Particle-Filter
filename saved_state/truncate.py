import os

script_dir = os.path.dirname(os.path.abspath(__file__))

for filename in os.listdir(script_dir):
    path = os.path.join(script_dir, filename)

    if filename.endswith(".txt") and os.path.isfile(path):
        with open(path, "w") as f:
            pass
        print(f"Cleared: {filename}")
