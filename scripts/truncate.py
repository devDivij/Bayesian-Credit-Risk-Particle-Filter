import os

print("WARNING: This script will clear existing saved states.")
choice = input("Do you want to continue? (y/n): ").lower()

if choice == 'y':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    sibling_dir = os.path.join(os.path.dirname(script_dir), 'saved_state')

    for filename in os.listdir(sibling_dir):
        path = os.path.join(sibling_dir, filename)

        if filename.endswith(".txt") and os.path.isfile(path):
            with open(path, "w") as f:
                pass
    print("All saved_state files have been cleared.")
else:
    print("Operation cancelled. No files were modified.")
