#!/bin/python3
import argparse
from dataclasses import dataclass


@dataclass
class Arguments:
    files: list[str]
    output: str
    replace_dots: bool


parser = argparse.ArgumentParser(
    "headerconcat",
    description="concats multiple headers into one",
)

_ = parser.add_argument(
    "files",
    nargs="+",
    help="input headers",
)

_ = parser.add_argument(
    "-o",
    "--output",
    dest="output",
    help="output header name",
)

_ = parser.add_argument(
    "--replace-dots",
    dest="replace_dots",
    help="replaces dots with underscores",
    default=False,
    action="store_true",
)

args = parser.parse_args(namespace=Arguments)

with open(args.output, "w") as output:
    # Normalised header containing stdint since generated header uses uint32_t
    output.writelines(
        [
            "#pragma once\n",
            "#include <stdint.h>\n",
        ]
    )

    for file in args.files:
        with open(file, "r") as infile:
            # Ignore the first two lines to remove header guard and comment, replace dots if neccessary
            output.writelines(
                (x.replace(".", "_") for x in infile.readlines()[2:])
                if args.replace_dots
                else infile.readlines()[2:]
            )
