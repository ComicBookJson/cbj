# ComicBookJson C++ SDK

Universal C++ API for reading, editing and converting comic books.

## Core API

- `Cbj::Open()` auto-detects `.cbjz`, `.cbz`, `.cbr`, `.cb7`, `.cbt` and `.pdf`.
- `GetPage()` and `GetPages()` materialize only requested pages.
- CBJZ indexing scans the JSON stream and stores byte offsets; the full Base64 payload is never loaded at open time.
- `GetPagesForView()` supports Single, Desktop and Mobile access patterns.
- `SetCachePages()` controls the page cache radius.
- `SaveAsCbjz()` writes CBJZ incrementally, one page at a time.
- `CbjBuilder` creates new CBJZ documents.
- `StringImageHelper` converts image bytes to/from Base64.

The CBJZ implementation follows the v1 schema from the `ComicBookJson/schema` repository.

## SWIG

The language-neutral SWIG interface is in `swig/cbj.i`. It exposes the public C++ SDK to generated bindings; the target language/runtime can be selected with the normal SWIG command line.

## Memory model

Opening a large CBJZ does not deserialize the `pages` array. Only structural metadata and small page indexes are retained. A requested page loads one page object, including its Base64 image, and the configurable cache controls how many nearby page objects remain resident.
