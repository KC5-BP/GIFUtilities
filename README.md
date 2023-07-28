# gifDecoderInC **(WIP)**

C Module to extract GIF informations &amp; decode datas into RGB array.

References:

- [GIF Picture Format](https://en.wikipedia.org/wiki/GIF#Example_GIF_file)

- [GIF Animation Format](https://en.wikipedia.org/wiki/GIF#Animated_GIF)

- [GIF Format in depth (ver. 89a)](https://www.w3.org/Graphics/GIF/spec-gif89a.txt)

After some thoughts, the idea here is to have 3 modules for the common part (Header, Logical Screen Descriptor and Global Color Table)

In order to limite the number of access to same place, one module will retrieve both the structure and content of the file.

Thus, 2 sub-modules will retrieve only one of each part: Structure OR Content

