#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
from cpt.packager import ConanMultiPackager
from cpt.ci_manager import CIManager
from cpt.printer import Printer


class BuilderSettings(object):

    @property
    def branch(self):
        """ Get branch name
        """
        printer = Printer(None)
        ci_manager = CIManager(printer)
        return ci_manager.get_branch()

    @property
    def username(self):
        """ Set BehaviorTree as package's owner
        """
        return os.getenv("CONAN_USERNAME", "BehaviorTree")

    @property
    def upload(self):
        """ Set BehaviorTree repository to be used on upload.
            The upload server address could be customized by env var
            CONAN_UPLOAD. If not defined, the method will check the branch name.
            Only master or CONAN_STABLE_BRANCH_PATTERN will be accepted.
            The master branch will be pushed to testing channel, because it does
            not match the stable pattern. Otherwise it will upload to stable
            channel.
        """
        if os.getenv("CONAN_UPLOAD", None) is not None:
            return os.getenv("CONAN_UPLOAD")

        prog = re.compile(self.stable_branch_pattern)
        if self.branch and prog.match(self.branch):
            return "https://api.bintray.com/conan/BehaviorTree/conan"

        return None

    @property
    def upload_only_when_stable(self):
        """ Force to upload when match stable pattern branch
        """
        return os.getenv("CONAN_UPLOAD_ONLY_WHEN_STABLE", True)

    @property
    def stable_branch_pattern(self):
        """ Only upload the package the branch name is like a tag
        """
        return os.getenv("CONAN_STABLE_BRANCH_PATTERN", r"\d+\.\d+\.\d+")

    @property
    def version(self):
        return self.branch if re.match(self.stable_branch_pattern, self.branch) else "latest"

    @property
    def reference(self):
        """ Read project version from branch name to create Conan referece
        """
        return os.getenv("CONAN_REFERENCE", "BehaviorTree.CPP/{}".format(self.version))

if __name__ == "__main__":
    settings = BuilderSettings()
    builder = ConanMultiPackager(
        reference=settings.reference,
        username=settings.username,
        upload=settings.upload,
        upload_only_when_stable=settings.upload_only_when_stable,
        stable_branch_pattern=settings.stable_branch_pattern,
        test_folder=os.path.join("conan", "test_package"))
    builder.add_common_builds(pure_c=False)
    builder.run()
