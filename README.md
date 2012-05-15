# dsoquad-demo

Examples of using the features of the DSO Quad BIOS and MCU.

folder         | notes
---------------|---------------------------------------------------------------
`experiments1` | A single application that tries out many features
`font-demo`    | A demonstration of a proportional typeface in under 1000 bytes
`lib`          | Optional support code
`support`      | Essential support files such as linker scripts
`tiny-menu`    | Presents a UI to boot any page, not just app slots 1 through 4
`torch`        | A "torch" that turns the screen white and has two brightness settings

The [font_perf experiment](dsoquad-demo/blob/master/experiments1/experiments/font_perf.c) includes a text renderer that is over 3&times; as fast as the BIOS renderer using the same glyphs.
