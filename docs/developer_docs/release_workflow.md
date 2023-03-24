# libpymcr release workflow

The release workflow is mostly automated using continuous integration builds,
but some actions and triggering a release needs to be done manually by the developer.

To create a release.

1. Create a branch and edit the `CHANGELOG.md` and `CITATION.cff` files to update it with a new version number.
2. Create a new PR from the branch. The PR must have `RELEASE` in the title.
3. This will trigger a build with multiple versions of python.
4. Review the branch, check that all tests for all python versions pass and if so merge.
5. Once merged, the CI should trigger various actions to create a github release.
6. Check that the release page is correct (has all the necessary wheels and mex files, that the release notes are ok).
7. Then manually trigger the `Upload PyPI wheels` action to upload wheels to PyPI.


In particular, in step 1:

* in `CHANGELOG.md` the first title line must have the form:

```
# [<newver>](https://github.com/pace-neutrons/libpymcr/compare/<oldver>...<newver>)
```

* in `CITATION.cff` the `version` field must be updated with a new version

If the version string in these two files do not match,
or if the version string matches an existing git tag,
then the CI build will fail.

Step 7 is because PyPI releases cannot be changed once uploaded
(the release version string and information; wheels can still be added or removed)
so it is better not to trigger the upload automatically.
