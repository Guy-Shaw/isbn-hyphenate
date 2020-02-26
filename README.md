# isbn-hyphenate -- Hyphenate an ISBN using International ISBN Agency table

Many websites offer ISBN services, [1]
such as validation, conversion between ISBN-10 <-> ISBN-13,
and hyphenation.

So why offer yet another ISBN tool?

Two reasons:

1. All the services I have seen offer overly simplistic,
out of date answers.  They use regular expressions.
They are 80% correct solutions, which is maybe good
enough in a "Worse is Better" kind of way.

2. They are all TaaS -- Tools-as-a-Service,
which is fine, except when that is not what you want. [2]

`isbn-hyphenate` works directly with the XML rules table
published by the International ISBN Agency,
which is the authoritative source for information about
ISBN numbers.

  `http://www.isbn-international.org`

From that XML table, `isbn-hyphenate` produces code
that runs locally, completely self-contained,
able to work offline, and small enough for the entire
table to fit in a very small amount of memory.

## Options and commands

So far, there is very little in the way of commands,
or subcommands or options.

The main() program is just a primitive test bench.
It just takes two arguments:
the file name of the XML table;
and an ISBN-13.

It prints out the hyphenated ISBN-13.
That's it.

In the process it sends out a lot of debug information:
the prefix table, the agencies, and the rules tables.

## Plans for the future

`isbn-hyphenate` is intended to be component in a larger
project, `libisbn`, which would do the full range of
validation, conversion, etc., as well as hyphenation.

All of that library would adhere to the design
goals of

  1) being correct, using authoritative data, keeping up-to-date; and

  2) being fast, compact, and self-contained, capable of working
off-line on a small device.

## Implementation

ISBN prefixes are variable length.
Using regular expressions has its problems.
A correct lookup of prefixes could be achieved
by arranging a list of prefixes in order of length [3],
then scanning longest prefixes first.
That would work, but is inefficient in space and time.

Trying to do a binary search on a list of prefixes,
as opposed to a list of exact matches, can be tricky.
Trying to search for prefixes in a hash table
has the same sort of problem.

`isbn-hyphenate` converts all the prefixes into
a finite-state transducer (FST), in which the final state
yields an index into the rules table.
This is compact, fast, and pretty easy to understand.
It is not that much more complex than a linear search.

An FST, loaded with all the prefixes, can be interrogated
for prefix matches.  It need not require a full match of the
given 'key', just a prefix match at a final state.


## Notes

[1]
Example websites:

  `http://pcn.loc.gov/isbncnvt.html`
  Library of Congress
  PCN := Preassigned Control Number Program

  Problem: underlying JavaScript code uses
  hard-code function consisting of a mix of
  regular exoressions and range checks.


  `https://www.isbn.org/ISBN_converter`
  Problem: Claims to only work for USA and Australia.
  Problem: Uses JQuery on a non-public set of data.


  `https://www.pearson.com/us/isbn-converter.html`
  Problem: non-public data.
  Problem: does not hyphenate.

[2]
Microservices, in general tend to be rather opaque,
and full of dependencies.
Opaque, not in the way that abstraction is a good thing
because it is the best way to reason about complexity.
No, just murky, with some implementation details hidden,
and subject to change, not under ones control.
And full of dependencies on being online and having a lot
of scaffolding, etc. working all the time.
A simple ISBN hyphenator as a web service can be slow,
unreachable at any time, can be permanently removed.
The communication overhead typically dwarfs the actual
computation.  Trying to follow link to find out where the
real underlying logic can be very tedious.


[3]
Of course some hybrid approach is feasible.
The entire set of prefixes can be partitioned by length;
then any number of efficient data structures and search methods
can be used on each of the sets of fixed-length prefixes,
and they would be probed in order.  But, an FST, overall,
is faster and simpler, in my opinion.

## License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

##

-- Guy Shaw

   gshaw@acm.org

