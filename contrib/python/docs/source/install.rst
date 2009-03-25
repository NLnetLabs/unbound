Installation
===================================

**Prerequisites**

Python 2.4 or higher, SWIG 1.3 or higher, GNU make

**Download**

You can download the source codes `here`_.
The latest release is 1.1.1, Jan 15, 2009.

.. _here: unbound-1.1.1-py.tar.gz

**Compiling**

After downloading, you can compile the pyUnbound library by doing::

	> tar -xzf unbound-1.1.1-py.tar.gz
	> cd unbound-1.1.1
	> ./configure --with-pyunbound
	> make

You need GNU make to compile sources; SWIG and Python devel libraries to compile extension module. 


**Testing**

If the compilation is successfull, you can test the python LDNS extension module by::

	> cd contrib/python
	> make testenv
	> ./dns-lookup.py

In contrib/examples you can find simple applications written in Python using the Unbound extension.
