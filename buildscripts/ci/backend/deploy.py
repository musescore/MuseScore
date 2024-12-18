#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import argparse
import sys

from jenkinsapi.jenkins import Jenkins

jenkins_url = "https://jenkins.mu.se/"
user = "e.ismailzada" # todo

build_job_name = "build_goosar"
deploy_job_name = "deploy_goosar"

def wait_for_job_to_finish(job, params):
    qi = job.invoke(build_params=params)

    # Block this script until build is finished
    if qi.is_queued() or qi.is_running():
      qi.block_until_complete()

    build = qi.get_build()
    print(build)

    return build.is_good()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Deploy to musescore.com")

    parser.add_argument("--mu_version", type=str, help="MuseScore version", required=True)
    parser.add_argument("--api_token", type=str, help="API Token", required=True)
    args = parser.parse_args()

    build_params = {"MS_VERSION": args.mu_version}
    api_token = args.api_token

    jenkins = Jenkins(jenkins_url, user, api_token)

    # Trigger the build
    jenkins.build_job(build_job_name, build_params)
    build_job = jenkins[build_job_name]
    if wait_for_job_to_finish(build_job, build_params):
        print("Build finished")
    else:
        print("Build failed")
        sys.exit(1)

    # Trigger the deploy
    jenkins.build_job(deploy_job_name, build_params)
    deploy_job = jenkins[deploy_job_name]
    if wait_for_job_to_finish(deploy_job, build_params):
        print("Deploy finished")
    else:
        print("Deploy failed")
        sys.exit(1)
