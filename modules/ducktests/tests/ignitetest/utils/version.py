# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Module contains ignite version utility class.
"""

from distutils.version import LooseVersion

from ignitetest import __version__


class IgniteVersion(LooseVersion):
    """
    Container for Ignite versions which makes versions simple to compare.

    distutils.version.LooseVersion (and StrictVersion) has robust comparison and ordering logic.

    Example:

        v27 = IgniteVersion("2.7.0")
        v28 = IgniteVersion("2.8.1")
        assert v28 > v27  # assertion passes!
    """
    def __init__(self, version_string):
        self.is_dev = (version_string.lower() == "dev")
        if self.is_dev:
            version_string = __version__

            # Drop dev suffix if present
            dev_suffix_index = version_string.find(".dev")
            if dev_suffix_index >= 0:
                version_string = version_string[:dev_suffix_index]

        super().__init__(version_string)

    def __str__(self):
        if self.is_dev:
            return "dev"

        return super().__str__()

    def __repr__(self):
        return "IgniteVersion ('%s')" % str(self)


DEV_BRANCH = IgniteVersion("dev")

# 2.7.x versions
V_2_7_6 = IgniteVersion("2.7.6")
LATEST_2_7 = V_2_7_6

# 2.8.x versions
V_2_8_0 = IgniteVersion("2.8.0")
V_2_8_1 = IgniteVersion("2.8.1")
LATEST_2_8 = V_2_8_1

# 2.9.x versions
V_2_9_0 = IgniteVersion("2.9.0")
V_2_9_1 = IgniteVersion("2.9.1")
LATEST_2_9 = V_2_9_1

# 2.10.x versions
V_2_10_0 = IgniteVersion("2.10.0")
LATEST_2_10 = V_2_10_0

LATEST = LATEST_2_10
