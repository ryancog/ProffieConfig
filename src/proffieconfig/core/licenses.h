/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/core/licenses.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils/types.h"

struct LicenseInfo{
    cstring name_;
    cstring detail_;
    cstring date_;
    // Really, copyright holder...
    cstring author_;
    cstring license_;
};

constexpr array<LicenseInfo, 6> LICENSES{{
    {
        .name_= "ProffieConfig",
        .detail_="The All-In-One Proffieboard Management Utility",
        .date_="2023-2026",
        .author_="Ryan Ogurek",
        .license_=
        "This program is free software: you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation, either version 3 of the License, or\n"
        "(at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program. If not, see https://www.gnu.org/licenses/."
    },
    {
        .name_="wxWidgets",
        .detail_=
        "A C++ library that lets developers create applications for Windows, macOS,\n"
        "Linux and other platforms with a single code base.",
        .date_="1992-2026",
        .author_="wxWidgets team",
        .license_=
        "             wxWindows Library Licence, Version 3.1\n"
        "              ======================================\n"
        
        "Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al\n"
        
        "Everyone is permitted to copy and distribute verbatim copies\n"
        "of this licence document, but changing it is not allowed.\n"
        
        "                     WXWINDOWS LIBRARY LICENCE\n"
        "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n"
        
        "This library is free software; you can redistribute it and/or modify it\n"
        "under the terms of the GNU Library General Public Licence as published by\n"
        "the Free Software Foundation; either version 2 of the Licence, or (at\n"
        "your option) any later version.\n"
        
        "This library is distributed in the hope that it will be useful, but\n"
        "WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library\n"
        "General Public Licence for more details.\n"
        
        "You should have received a copy of the GNU Library General Public Licence\n"
        "along with this software, usually in a file named COPYING.LIB.  If not,\n"
        "write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
        "Boston, MA 02110-1301 USA.\n"
        
        "EXCEPTION NOTICE\n"
        
        "1. As a special exception, the copyright holders of this library give\n"
        "permission for additional uses of the text contained in this release of\n"
        "the library as licenced under the wxWindows Library Licence, applying\n"
        "either version 3.1 of the Licence, or (at your option) any later version of\n"
        "the Licence as published by the copyright holders of version\n"
        "3.1 of the Licence document.\n"
        
        "2. The exception is that you may use, copy, link, modify and distribute\n"
        "under your own terms, binary object code versions of works based\n"
        "on the Library.\n"
        
        "3. If you copy code from files distributed under the terms of the GNU\n"
        "General Public Licence or the GNU Library General Public Licence into a\n"
        "copy of this library, as this licence permits, the exception does not\n"
        "apply to the code that you add in this way.  To avoid misleading anyone as\n"
        "to the status of such modified files, you must delete this exception\n"
        "notice from such code and/or adjust the licensing conditions notice\n"
        "accordingly.\n"
        
        "4. If you write modifications of your own for this library, it is your\n"
        "choice whether to permit this exception to apply to your modifications.\n"
        "If you do not wish that, you must delete the exception notice from such\n"
        "code and/or adjust the licensing conditions notice accordingly."
    },
    {
        .name_="LibTomCrypt",
        .detail_="",
        .date_="",
        .author_="Team libtom",
        .license_=
        "                          The LibTom license\n"
        "\n"
        "This is free and unencumbered software released into the public domain.\n"
        "\n"
        "Anyone is free to copy, modify, publish, use, compile, sell, or\n"
        "distribute this software, either in source code form or as a compiled\n"
        "binary, for any purpose, commercial or non-commercial, and by any\n"
        "means.\n"
        "\n"
        "In jurisdictions that recognize copyright laws, the author or authors\n"
        "of this software dedicate any and all copyright interest in the\n"
        "software to the public domain. We make this dedication for the benefit\n"
        "of the public at large and to the detriment of our heirs and\n"
        "successors. We intend this dedication to be an overt act of\n"
        "relinquishment in perpetuity of all present and future rights to this\n"
        "software under copyright law.\n"
        "\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
        "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
        "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
        "IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR\n"
        "OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n"
        "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
        "OTHER DEALINGS IN THE SOFTWARE.\n"
        "\n"
        "For more information, please refer to <http://unlicense.org/>"
    },
    {
        .name_="libbacktrace",
        .detail_=
        "A C library that may be linked into a C/C++ program to produce symbolic\n"
        "backtraces. Initially written by Ian Lance Taylor iant@golang.org.",
        .date_="2012-2024",
        .author_="Free Software Foundation, Inc.",
        .license_=
        "Redistribution and use in source and binary forms, with or without\n"
        "modification, are permitted provided that the following conditions are\n"
        "met:\n"
        "\n"
        "    (1) Redistributions of source code must retain the above copyright\n"
        "    notice, this list of conditions and the following disclaimer.\n"
        "\n"
        "    (2) Redistributions in binary form must reproduce the above copyright\n"
        "    notice, this list of conditions and the following disclaimer in\n"
        "    the documentation and/or other materials provided with the\n"
        "    distribution.\n"
        "\n"
        "    (3) The name of the author may not be used to\n"
        "    endorse or promote products derived from this software without\n"
        "    specific prior written permission.\n"
        "\n"
        "THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
        "IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
        "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
        "DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,\n"
        "INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
        "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
        "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
        "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\n"
        "STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING\n"
        "IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n"
        "POSSIBILITY OF SUCH DAMAGE."
    },
    {
        .name_="Arduino CLI",
        .detail_=
        "An all-in-one solution that provides Boards/Library Managers, sketch\n"
        "builder, board detection, uploader, and many other tools needed to use any\n"
        "Arduino compatible board and platform from command line or machine\n"
        "interfaces.",
        .date_="2018-2026",
        .author_="Arduino SA and contributors",
        .license_=
        "This program is free software: you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation, either version 3 of the License, or\n"
        "(at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n"
    },
    {
        .name_="ProffieOS",
        .detail_=
        "Control software for lightsabers and other props.\n"
        "http://fredrik.hubbe.net/lightsaber/teensy_saber.html",
        .date_="2016-2026",
        .author_="Fredrik Hubinette et al.",
        .license_=
        "This program is free software: you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation, either version 3 of the License, or\n"
        "(at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program.  If not, see <http://www.gnu.org/licenses/>."
    },
}};

