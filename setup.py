from distutils.core import setup, Extension

module = Extension('caenrfidc',
		libraries	= ['msgbuff', 'caenrfid'],
		sources		= ['caenrfidcmodule.c'],
	)

setup (name = 'caenrfidcn',
		version		= '0.10.0',
		description	= 'CAENRFID client support.',
		maintainer	= "Rodolfo Giometti",
		maintainer_email= "giometti@cosino",
		license		= "GPLv2",
		url		= "https://github.com/cosino/caenrfidc-python",
		ext_modules	= [module],
	)
