# Copyright 2014 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'target_defaults': {
    'include_dirs': [
      '../..',
    ],
  },
  'targets': [
    {
      'target_name': 'crash_report',
      'type': 'executable',
      'sources': [
        'crash_report/crash_report.mm',
        'crash_report/on_demand_symbol_supplier.h',
        'crash_report/on_demand_symbol_supplier.mm',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
        ],
      },
      'dependencies': [
        '../common/common.gyp:common',
        '../processor/processor.gyp:processor',
      ],
    },
    {
      'target_name': 'dump_syms',
      'type': 'executable',
      'sources': [
        'dump_syms/dump_syms_tool.cc',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
        ],
      },
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'macho_dump',
      'type': 'executable',
      'sources': [
        'dump_syms/macho_dump.cc',
      ],
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'minidump_upload',
      'type': 'executable',
      'sources': [
        'symupload/minidump_upload.m',
      ],
      'include_dirs': [
        '../../common/mac',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
        ],
      },
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'symupload',
      'type': 'executable',
      'sources': [
        'symupload/symupload.m',
      ],
      'include_dirs': [
        '../../common/mac',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
        ],
      },
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
  ],
}
