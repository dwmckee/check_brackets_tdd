project('check_brackets', 'cpp',
        default_options: ['cpp_std=c++20'],
        version: '0.1.0',
        license: 'MIT')

doctest_proj = subproject('doctest')
doctest_dep = doctest_proj.get_variable('doctest_dep')
# NB: Doctest dependency is needed on anything that includes doctest.h even if
# we use DOCTEST_CONFIG_DISABLE

test_exe = executable('test-check-brackets', ['test-main.cpp'],
                      dependencies: doctest_dep)
test('unit tests', test_exe, args: [])
