# Simple Linux Shell

A barebone shell for Linux.

I'm doing this project to learn more C and to better understand how Linux systems work under the hood.

Has built-in commands which you can see with "help" but supports installed binaries too.

Dependecies required: libedit-dev

Debian/Ubuntu:
```
sudo apt update
sudo apt install libedit-dev
```

Fedora/RHEL/CentOS:
```
sudo dnf install libedit-devel
```

openSUSE:
```
sudo zypper refresh
sudo zypper install libedit-devel
```

Arch Linux/Manjaro:
```
sudo pacman -Sy libedit
```

Steps to use:
1. Compile the shell with:
```
make
```
2. Run it through a terminal (i am testing with xterm):
```
xterm ./shell
```
