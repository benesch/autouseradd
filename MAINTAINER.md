# Maintainer Handbook

To cut a new release:

- [ ] Choose a semver-compatible version of the form `X.Y.Z`, without a leading
      `v`.
- [ ] Update the version in:
  - [ ] README.md
  - [ ] CHANGELOG.md
  - [ ] autouseradd-suid.c
  - [ ] autouseradd.1
- [ ] Push a new tag:
  ```shell
  $ git tag -a $VERSION -m $VERSION && git push origin $VERSION
  ```
