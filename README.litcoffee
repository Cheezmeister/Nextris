#!/usr/bin/env coffee

Hey there. Thanks for checking out my little bit of hackery.

Nextris is a synaesthetic, portable, malleable Tetris clone with procedural graphics and audio.  In other words, there are no sprites, and there are no soundbites.  Everything's done on the fly!

By default, movement uses WASD and rotation uses the L and P keys, but you can change this by hitting Return/Enter during play and following the instructions in the console, or alternatively editing keyconfig.txt and substituting the appropriate ASCII values.
Have fun!

By the way, because I am an incredible dork, this readme also happens to be a script by which you may launch the program.
It's written in [Literate CoffeeScript](http://coffeescript.org/#literate), which I think is way cool.
You can run it if you have nodejs with CoffeeScript, but you don't (and shouldn't) have to. 

`$ coffee README.litcoffee help`

## 1. Usage

The game is designed with flexibility in mind. We will default to classic Tetris rules,
but as many gameplay params as I can manage shall be tweakable. This chunk isn't used
anywhere yet; I'm keeping it around for reference.

    usage = """ Usage: $0 [options]

    -w Run in Windowed mode

    --keys.left=A | Set the directional buttons | default WASD
    --keys.right=D
    --keys.up=W
    --keys.down=S
    --keys.rotleft
    --keys.rotright
    --keys.pause=SPACE | Set the pause button | default spacebar

    --game.width=10 | Set the field width in blocks
    --game.height=20 | Set the field height in blocks
    --game.speed=20Hz | Gameplay speed | default 20Hz i.e. 1200bpm | default 20.0f
    --game.instadrop=on | Enable or disable insta-dropping with "up" | default on
    --game.lineclear=on | Enable or disable classic lineclearing | default on
    --game.colorclear=off | Enable or disable colorclearing | default off
    --game.colorthreshold=4 | Set the minimum number of blocks needed to colorclear | default 4
    --game.lookahead=N | See the next N blocks in your queue | default 1

    --graphics.windowed Run in Windowed mode
    --graphics.maxparticles |
    --graphics.framerate | Sets framerate as a multiplier
    """

### Options!

First let's set up the default options. These won't get passed into the game directly 
and so changing them here will have no effect unless you write out a `.nextris.conf` 
with `writeconf`. The ultimate defaults can be found in `src/options.cpp`.

    options =
      keys:
        up:   'w'
        left: 'a'
        down: 's'
        right:'d'
        rotleft: 'l'
        rotright: 'p'
        pause: ' '
      game:
        width:  10
        height: 20
        speed:  20 # in Hz
        instadrop:  on
        lineclear:  on
        colorclear: off
        colorthreshold: 5
      graphics:
        windowed: false
        maxparticles: 200
        mult: 3
      audio:
        mult: 7
        track:
          pad:        on
          bassline:   on
          melody:     on
        fx:
          clear:      on
          drop:       on

I use a tree-ish, object-y structure for gameplay params here, but I may want
to flatten them out for simplicity in the conf file and elsewhere.

    flatten = (obj, prefix) ->
      prefix ?= ''
      switch typeof obj
        when 'object'
          (flatten val, "#{prefix}#{if prefix then '.' else ''}#{key}" for key, val of obj).join "\n"
        else "#{prefix} = #{obj}"

## 2. Lame, Boring Setup

Skip this section unless you're as nerdy as me.

### Aliases and Idioms

I use console.log enough that this seems to make sense. I think.

    _ = console.log

Putting app constants in one namespace seems reasonable.

    C =
      CONF_FILE: '.nextris.conf'

Same goes for args. That argv array starting at [2] funkiness is redonk.

    A =
      interpreter: process.argv[0]
      thisfile: process.argv[1]
      command: process.argv[2]
      rest: process.argv[3..]

Shorthand for shelling out to system. This might be dangerous, but #yolo.

    cp = require 'child_process'
    exec = cp.exec
    spawn = cp.spawn

    $ = (str) ->
      exec str, (error, stdout, stderr) ->
        _ stdout

Please be careful and don't [arémaréf](https://twitter.com/NeoCheez/status/515723127796809728)
yourself.

    # Uncomment me if you dare
    # $ "cat #{process.argv[1]}"  # Hey look, I'm a quine!


But enough farting about. Let's define some silly relengy helpers.

## 3. Build System

Nominally, the build system is [CMake][]. CMake is great at building Cxx
portably, but it gets messy fast with more complex build automation.
Makefiles are for your granddad and shell scripts are frigging
impossible to read.

We're going to define some actions you can take via this franken-script.

    actions =

### Bootstrapping

This thang is mad portable. I tend to use the four configs below, though
you can just as happily make a build/ folder and call it a day.

      bootstrap:
        run: ->
          configs =
            'build/native/debug': ''
            'build/native/release': '-DCMAKE_BUILD_TYPE=Release'
            'build/js/debug': '-DUSE_EMSCRIPTEN=1'
            'build/js/release': '-DUSE_EMSCRIPTEN=1 -DCMAKE_BUILD_TYPE=Release'

          for cfg, options of configs
            $ "mkdir -p #{cfg}"
            $ "cd #{cfg} && cmake ../../.. #{options}"

### Writing configuration

I'm a lazy doof so I love when I can start with a nice stub config/dotfile.
Call `writeconf` to write the default options.

      writeconf:
        run: ->
          fs = require 'fs'
          path = require 'path'

We're going to write out a conf file in your homedir, but first need
to figure out where to put it.

          homedir = process.env.HOME || process.env.HOMEPATH || process.env.USERPROFILE
          filename = A.rest.first || (path.join homedir, C.CONF_FILE)

Write the values.

          fs.writeFile filename, flatten(options), (err) -> if (err) then _ "Couldn't write: #{err}"


### Self-Documenting

Inception, etc.

      doc:
        run: ->
          $ "markdown #{A.thisfile} > README.html"

Usage.

      help:
        run: (self) ->
          actions = [name for name of self]
          _ "Usage: #{A.thisfile} [action]\n"
          _ "Actions: \n#{actions.join('\n')}\n"
          _ "Default options:"
          _ flatten(options)

Echo options.

      options:
        run: ->
          _ flatten(options)

### Releasing

      release:
        run: (self) ->
          self.bootstrap.run()
          $ 'cmake --build build/native/release'
          $ 'cmake --build build/js/release'


### Do It

    if A.command of actions
        return (actions[A.command].run)(actions)

## 4. Game Time

### Running

Finally! 

    actions.bootstrap.run()
    $ 'cd build/native/debug && make && ./nextris*'

## Hacking

First, please bear in mind that development on this thing spans back quite a 
few years, so you're liable to run into old moldy corners that I can't fathom
but have worked since I wrote them. You're on your own there.

Second, see [bootstrapping](#bootstrapping), above. You'll also need the following:

- SDL 1.2
- PortAudio (optional)
- Emscripten (optional)

Third, have fun and be sure to share any improvements.

[CMake]: http://www.cmake.org
