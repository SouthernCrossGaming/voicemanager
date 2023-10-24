#!/usr/bin/env python
# vim: set ts=2 sw=2 tw=99 et:

import sys

def detect_distutils():
    sys.path.pop(0)
    try:
        import ambuild2.util
        try:
            val = getattr(ambuild2.util, 'INSTALLED_BY_PIP_OR_SETUPTOOLS')
        except AttributeError:
            sys.exit(1)
    except ImportError:
        pass

    sys.exit(0)

# This if statement is supposedly required by multiprocessing.
if __name__ == '__main__':
    from setuptools import setup, find_packages
    try:
        import sqlite3
    except:
        raise SystemError('py-sqlite3 must be installed')

    amb_scripts = []
    if sys.platform != 'win32':
        if sys.platform == 'darwin':
            amb_scripts.append('scripts/ambuild_dsymutil_wrapper.sh')
        else:
            amb_scripts.append('scripts/ambuild_objcopy_wrapper.sh')

    setup(name = 'AMBuild',
          version = '2.0',
          description = 'AlliedModders Build System',
          author = 'David Anderson',
          author_email = 'dvander@alliedmods.net',
          url = 'http://www.alliedmods.net/ambuild',
          packages = find_packages(),
          python_requires = '>=3.3',
          entry_points = {'console_scripts': ['ambuild = ambuild2.run:cli_run']},
          scripts = amb_scripts,
          zip_safe = False)
