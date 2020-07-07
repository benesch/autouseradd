# Changelog

All notable changes to autouseradd are documented in this file. The format is
based on [Keep a Changelog].

autouseradd adheres to [Semantic Versioning].

## Unreleased

* Preserve environment variables that the glibc loader automatically strips from
  the environment when executing a setuid binary like autouseradd. The most
  notable such environment variable is `TMPDIR`. As a side effect, autouseradd
  learned a new option, `-e NAME=VALUE`, which sets the environment variable
  `NAME` to `VALUE`.

  For more details about the environment variables stripped by the loader,
  see the references to "secure-execution mode" in the [ld.so man page][ld.so].

  [ld.so]: https://man7.org/linux/man-pages/man8/ld.so.8.html

* Learn to print help when the `--help` option, or its short form `-h`, is
  specified.

## [1.1.0] / 2018-07-31

Learn to skip creating a home directory when the `--no-create-home` option, or
its short form, `-M`, is specified.

## 1.0.0 / 2017-11-07

Initial release.

[Keep a Changelog]: http://keepachangelog.com/en/1.0.0/
[Semantic Versioning]: http://semver.org/spec/v2.0.0.html
[1.1.0]: https://github.com/benesch/autouseradd/compare/1.0.0...1.1.0
