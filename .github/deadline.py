import os
import sys
from github import Github
from datetime import datetime
from zoneinfo import ZoneInfo


def main():
    moscow_tz = ZoneInfo("Europe/Moscow")
    current_date = datetime.now(moscow_tz)
    deadline_date = {
        "lab:clang": datetime(2025, 3, 19, hour=19, tzinfo=moscow_tz),
        "lab:llvm ir": datetime(2025, 4, 9, hour=19, tzinfo=moscow_tz),
        "lab:backend": datetime(2025, 5, 8, hour=19, tzinfo=moscow_tz),
        "lab:mlir": datetime(2025, 5, 21, hour=19, tzinfo=moscow_tz),
    }

    gh = Github(os.environ["GITHUB_TOKEN"])
    repo = gh.get_repo(os.environ["GITHUB_REPOSITORY"])
    pr = repo.get_pull(int(os.environ["PR_NUMBER"]))
    labels = ["lab:clang", "lab:llvm ir", "lab:backend", "lab:mlir"]
    lab_label = None

    for label in pr.get_labels():
        if label.name in labels:
            lab_label = label.name

    if not lab_label:
        return

    if current_date > deadline_date[lab_label]:
        pr.add_to_labels("delayed")


if __name__ == "__main__":
    sys.exit(main())
