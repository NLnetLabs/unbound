#!/usr/bin/env bash

# This step should install tools needed for all packages - OpenSSL, Expat and Unbound
echo "Updating tools"
brew update 1>/dev/null
echo "Installing tools"
brew install autoconf automake libtool pkgconfig curl perl 1>/dev/null
