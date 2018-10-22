// Copyright (c) 2018, The TurtleCoin Developers
// Copyright (c) 2018, 2ACoin Developers
//
// Please see the included LICENSE file for more information

#pragma once

const std::string windowsAsciiArt =
      "\n \n"
      "                           .===;========.__, \n"
      "                           (/__)___________| \n"
      "     L___________,--,--/ /-,-,-/ /-,   ________ \n"
      "=====)o o o o ======== )) ____,===,___""" "7_/_,_,_,_,'---,-, \n"
      "     `--._,_,_,-,--,--'' (____| _ |___ oo ; ; ; ; ; ;_____ T| \n"
      "              `-'--'-/_,-------| ) ___--,__,------._  __  |I| \n"
      "                        ==----/    / )/--/_         `-._`-'I| \n"
      "                       /=[  ]/     ` ==.- -             `-.L| \n"
      "                      /==---/            - -  \n"
      "                      '-.__/              __7 \n";

const std::string nonWindowsAsciiArt = 
      "\n \n"
      "                           .===;========.__, \n"
      "                           (/__)___________| \n"
      "     L___________,--,--/ /-,-,-/ /-,   ________ \n"
      "=====)o o o o ======== )) ____,===,___""" "7_/_,_,_,_,'---,-, \n"
      "     `--._,_,_,-,--,--'' (____| _ |___ oo ; ; ; ; ; ;_____ T| \n"
      "              `-'--'-/_,-------| ) ___--,__,------._  __  |I| \n"
      "                        ==----/    / )/--/_         `-._`-'I| \n"
      "                       /=[  ]/     ` ==.- -             `-.L| \n"
      "                      /==---/            - -  \n"
      "                      '-.__/              __7 \n";

/* Windows has some characters it won't display in a terminal. If your ascii
   art works fine on Windows and Linux terminals, just replace 'asciiArt' with
   the art itself, and remove these two #ifdefs and above ascii arts */
#ifdef _WIN32
const std::string asciiArt = windowsAsciiArt;
#else
const std::string asciiArt = nonWindowsAsciiArt;
#endif
