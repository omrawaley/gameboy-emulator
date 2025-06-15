/*
    Copyright (c) 2025 Om Rawaley

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "app.h"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "ERROR::CLI::NO_BOOT_ROM_PROVIDED\n");
    }
    if(argc < 3)
    {
        fprintf(stderr, "ERROR::CLI_NO_ROM_PROVIDED\n");
        exit(EXIT_FAILURE);
    }

    App app;
    app.start(argv[1], argv[2]);

    while(!app.quit)
    {
        app.update();
        app.draw();
    }
}