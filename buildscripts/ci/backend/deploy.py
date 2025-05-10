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
import time

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
    parser = argparse.ArgumentParser(description="Build and Deploy")

    parser.add_argument("--mu_version", type=str, help="MuseScore version", required=True)
    parser.add_argument("--deploy_stage", type=str, help="Deploy stage", required=True)
    parser.add_argument("--is_default", type=str, help="Should be used as default", default="false")
    parser.add_argument("--with_build", type=str, help="Build before deploy", default="true")
    parser.add_argument("--api_token", type=str, help="API Token", required=True)
    args = parser.parse_args()

    api_token = args.api_token

    jenkins = Jenkins(jenkins_url, user, api_token)

    if args.with_build == "true":
        # Trigger the build
        build_params = {
            "MS_VERSION": args.mu_version,
            "STAGE": args.deploy_stage
        }

        jenkins.build_job(build_job_name, build_params)
        build_job = jenkins[build_job_name]

        if wait_for_job_to_finish(build_job, build_params):
            print("Build finished")
        else:
            print("Build failed")
            sys.exit(1)

        # Give Jenkins time to complete its work
        time.sleep(5)

    # Trigger the deploy
    deploy_params = {
        "MS_VERSION": args.mu_version,
        "STAGE": args.deploy_stage,
        "DEFAULT": args.is_default
    }

    jenkins.build_job(deploy_job_name, deploy_params)

    deploy_job = jenkins[deploy_job_name]
    if wait_for_job_to_finish(deploy_job, deploy_params):
        print("Deploy finished")
    else:
        print("Deploy failed")
        sys.exit(1)
