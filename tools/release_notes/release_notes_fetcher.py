#!/usr/bin/env python3
import argparse
import os
import re
import subprocess
import urllib.request

fix_pattern = re.compile("[Ff]ix #([0-9]{4,})")
node_url_pattern = "https://musescore.org/node/{node}"
changelog_line_issue = "* [#{node}]({url}): {title}"
changelog_line_other = "* {title}"

def git_log(git_dir, rev1, rev2):
    cwd = os.getcwd()
    os.chdir(git_dir)

    # %s for subject (message)
    git = subprocess.run(["git", "log", "--format=format:%s", "{}..{}".format(rev1, rev2)], capture_output=True, text=True)

    os.chdir(cwd)
    return git.stdout

def get_node_url(node):
    return node_url_pattern.format(node=node)

def get_node_info(node):
    node_url = get_node_url(node)

    try:
    	response = urllib.request.urlopen(node_url)
    except urllib.error.HTTPError as e:
        print('Issue not found! HTTP response code: ' + str(e.code))
        return {"node": node, "url": node_url, "title": "!!!!! HTTP " + str(e.code)}

    html = str(response.read())

    # We may need some more complex HTML parsing to get
    # more info but just for title regex is enough
    title_match = re.search("\<title\>(.*)\</title\>", html)

    if title_match:
        title = title_match.group(1)
        trailer_idx = title.rfind('|') # the expected page title format is "{node_title} | MuseScore"
        title = title[:trailer_idx].rstrip()
    else:
        title = None

    return { "node": node, "url": node_url, "title": title }

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract changelog from MuseScore repo")
    parser.add_argument("rev1", help="Revision (tag, branch name or commit hash) of the old version")
    parser.add_argument("rev2", help="Revision (tag, branch name or commit hash) of the new version")
    parser.add_argument("--git-dir", default=".", help="Path to the git repo directory")

    args = parser.parse_args()

    log = git_log(args.git_dir, args.rev1, args.rev2)
    msgs = log.split('\n')

    issues = set()
    non_issues = []

    for msg in msgs:
        issue = False
        for fix_match in re.finditer(fix_pattern, msg):
            if fix_match:
                issues.add(int(fix_match.group(1)))
                issue = True

        if not issue and not "Merge pull request #" in msg:
            non_issues.append(msg)

    print("Changes not directly related to issues:", len(non_issues))
    for msg in non_issues:
        print(changelog_line_other.format(title=msg))

    print()

    issues_info = map(get_node_info, issues)
    print("Fixed issues:", len(issues))
    for info in issues_info:
        print(changelog_line_issue.format(**info))
