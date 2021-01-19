from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'filedup'
    version = '0.1.0'

    generators = 'cmake_find_package'

    requires = [
        ('boost/1.74.0')
    ]

    default_options = (
        'boost:shared=False',
    )
