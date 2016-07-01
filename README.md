STREAMON PROTOTYPE COMPILE GUIDE
================================

StreaMON Prototype is actually based on Blockmon (see blockmon.README for more info)
and written in C++11 language.

This guide will assume that you've already downloaded the code from repository
somewhere in your PC.

Let's call this location $STREAMON_ROOT.

DEPENDENCIES REQUIREMENTS
-------------------------

The following packages are required to compile StreaMON:

- cmake
- g++
- libboost-dev
- libpcap-dev
- libzmq3-dev

On Ubuntu or other Debian-based distributions you can retreive packages the following way:

$ sudo apt-get update
$ sudo apt-get install <pkgs>

COMPATIBILITY
-------------
StreaMON should compile well in all major new distributions, however was tested on:
    - Debian 8.2 (Jessie) with GCC 4.9.2
    - Arch Linux with GCC 5.2.0 (at October 22th 2015).

HOW TO COMPILE
--------------

1) Enter in $STREAMON_ROOT folder and type the following command:
        $ cmake .
       
        (yes, with the dot, not a mistake)

2) After cmake do the following:
        $ make -j<number of cpu cores that your pc has+1>

        Example: make -j5 (if you have a 4-core CPU)

3) The compilation process starts and end after a while, if all goes well you should see the following message:

        Linking CXX executable blockmon
        [100%] Built target blockmon


OTHER NOTES ON COMPILATION
--------------------------

By default StreaMON is compiled with the following flags:

-std=c++11 -O3 -march=native -Wall -flto

You can adjust these as you prefere in CMakeLists.txt.

FLTO option is the Link-Time Optimization that reduce executable size removing not used code/functions, 
for instance with FLTO on the blockmon executable size goes from ~12M to ~900K.

As a cons the linking time increase.

=========
SEMAPHORE
=========

Open Source Alternative to Ansible Tower https://ansible-semaphore.github.io/semaphore

DEPENDENCIES REQUIREMENTS
-------------------------

Ansible: http://docs.ansible.com/ansible/intro_installation.html

INSTALLATION
------------

1. Install VirtualBox & Vagrant
2. Run `vagrant plugin install gatling-rsync-auto`
3. Run `vagrant up` to start the vagrant box
4. Run `vagrant gatling-rsync-auto` to synchronise changes from your local machine to vagrant

### Running semaphore inside vagrant

1. `vagrant ssh`, `cd /opt/semaphore`
2. `npm install`
3. `bower install`
4. `npm install -g nodemon`
5. `nodemon bin/semaphore`

## Initial Login

```
Email:			'admin@semaphore.local'
Password:		'CastawayLabs'
```

## License

MIT