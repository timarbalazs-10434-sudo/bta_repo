#log_file_path = "app.log"
#contentOutput = ""

def getLogAsText(log_file_path):
    try:
        with open(log_file_path, "r", encoding="utf-8") as file:
            content = file.read()
            contentOutput = content
    except FileNotFoundError:
        print("Log file not found.")
    except Exception as e:
        print("Error reading log file:", e)
        
getLogAsText(log_file_path)
