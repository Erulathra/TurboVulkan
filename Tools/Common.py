import os
import time

PROJECT_DIR = f"{os.path.dirname(__file__)}/.."
EXTERNAL_DIR = f"{PROJECT_DIR}/Content/External"
CACHE_DIR = f"{os.path.dirname(__file__)}/Cache"

last_report_time = 0

def show_download_progress(block_num, block_size, total_size):
    global last_report_time

    if time.perf_counter() - last_report_time > 1.:
        downloaded_mib = block_num * block_size / 2. ** 20
        total_mib = total_size / 2. ** 20
        print(f"Downloading {downloaded_mib / total_mib * 100.:.2f}% [{downloaded_mib:.2f}MiB/{total_mib:.2f}MiB]")
        last_report_time = time.perf_counter()
