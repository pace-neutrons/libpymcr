from IPython import get_ipython
from IPython.core import magic_arguments
from IPython.core.magic import Magics, magics_class, line_magic
from IPython.display import Image, display
import ipykernel

import threading
import ctypes
import time
import sys
import io
import os
try:
    from tempfile import TemporaryDirectory, TemporaryFile, NamedTemporaryFile
except ImportError:
    from backports.tempfile import TemporaryDirectory, TemporaryFile, NamedTemporaryFile

_magic_class_ref = None
_windowed_figs = []


def _save_visible_figs(interface):
    # Adds all visible figures to save list and close all others
    children = interface.call('eval', "arrayfun(@(h) h, get(0, 'Children'), 'UniformOutput', false);")
    if not children:
        return
    is_visible = interface.call('eval', "arrayfun(@(h) logical(get(h, 'Visible')), get(0, 'Children'));")
    if not isinstance(is_visible, list):
        is_visible = [is_visible]
    global _windowed_figs
    _windowed_figs = []
    for visible, chld in zip(is_visible, children):
        if visible:
            _windowed_figs.append(chld)
        else:
            interface.call('close', chld)


def _get_plot_figs(interface):
    # Returns a list of figures which are not amongst the saved figures
    children = interface.call('eval', "arrayfun(@(h) h, get(0, 'Children'), 'UniformOutput', false);")
    if not children:
        return []
    is_saved = [False]*len(children)
    for saved_figs in _windowed_figs:
        is_saved = [a or interface.call('eq', saved_figs, b) for a, b in zip(is_saved, children)]
    return [ch for saved, ch in zip(is_saved, children) if not saved]


# We will overload the `post_run_cell` event with this function
# That callback is a method of `EventManager` hence the `self` argument
def showPlot(self=None, result=None):
    # We use a global reference to the magics class to get the reference to Matlab interpreter
    # If it doesn't exist, we can't do anything, and assume the user is just using Python
    ip = get_ipython()
    if ip is None or _magic_class_ref is None:
        return
    if _magic_class_ref.m is None:
        try:
            from . import Matlab
            _magic_class_ref.m = Matlab()._interface
        except RuntimeError:
            return
    interface = _magic_class_ref.m
    if _magic_class_ref.plot_type != 'inline':
        # Get list of figures which are not hidden now and keep them in list
        _save_visible_figs(interface)
        return
    nfig = int(interface.call('numel', interface.call('get', 0, "children"))[0][0])
    if nfig == 0:
        return
    if _magic_class_ref.next_pars:
        width, height = (_magic_class_ref.next_pars[idx] for idx in ['width', 'height'])
    else:
        width, height = (_magic_class_ref.width, _magic_class_ref.height)
    filetype = 'png'
    with TemporaryDirectory() as tmpdir:
        for ii, child in enumerate(reversed(_get_plot_figs(interface))):
            fname = os.path.join(tmpdir, f'{ii}.{filetype}')
            try:
                interface.call('set', child, 'PaperPosition', [0.5, 0.5, width/300., height/300.])
                interface.call('set', child, 'PaperUnits', 'inches')
                interface.call('print', child, fname, f'-d{filetype}', '-r300')
            except Exception as ex0:
                print(f'Could not draw figure {ii} due to error:')
                ip.showtraceback()
            else:
                display(Image(filename=fname))
            finally:
                interface.call('close', child)
        interface.call('set', 0, 'defaultfigurevisible', 'off', nargout=0)
        if _magic_class_ref.next_pars:
            _magic_class_ref.next_pars = None


@magics_class
class MatlabMagics(Magics):
    """
    Class for IPython magics for interacting with Matlab

    It defines several magic functions:

    %matlab_plot_mode - sets up the plotting environment (default 'inline')
    %matlab_fig - defines the inline figure size for the next plot only
    """

    def __init__(self, shell, interface):
        super(MatlabMagics, self).__init__(shell)
        self.m = interface
        self.shell = get_ipython().__class__.__name__
        self.output = 'inline'
        self.plot_type = 'inline' if self.shell == 'ZMQInteractiveShell' \
                else 'windowed'
        self.width = 400
        self.height = 300
        self.next_pars = None
        global _magic_class_ref
        _magic_class_ref = self

    @line_magic
    @magic_arguments.magic_arguments()
    @magic_arguments.argument('plot_type', type=str, help="Matlab plot type, either: 'inline' or 'windowed'")
    @magic_arguments.argument('output', nargs='?', type=str, help="Matlab output, either: 'inline' or 'console'")
    @magic_arguments.argument('-w', '--width', type=int, help="Default figure width in pixels [def: 400]")
    @magic_arguments.argument('-h', '--height', type=int, help="Default figure height in pixels [def: 300]")
    def matlab_plot_mode(self, line):
        """Set up libpymcr to work with IPython notebooks
        
        Use this magic function to set the behaviour of Matlab programs Horace and SpinW in Python.
        You can specify how plots should appear: either 'inline' [default] or 'windowed'.
        You can also specify how Matlab text output from functions appear: 'inline' [default] or 'console'

        Examples
        --------
        By default the inline backend is used for both figures and outputs. 
        To switch behaviour use, use:

            In [1]: %matlab_plot_mode windowed             # windowed figures, output unchanged ('inline' default)
            In [2]: %matlab_plot_mode console              # figure unchanged ('inline' default), console output
            In [3]: %matlab_plot_mode windowed console     # windowed figures, console output
            In [4]: %matlab_plot_mode inline inline        # inline figures, inline output
            In [5]: %matlab_plot_mode inline               # inline figures, inline output
            In [6]: %matlab_plot_mode inline console       # inline figures, console output
            In [7]: %matlab_plot_mode windowed inline      # windowed figures, console output

        Note that if you specify `%matlab_plot_mode inline` this sets `'inline'` for _both_ figures and outputs.
        If you want inline figures and console outputs or windowed figures and inline output you must specify
        that specifically.

        Note that using (default) inline text output imposes a slight performance penalty.

        For inlined figures, you can also set the default figure size with

            In [8]: %matlab_plot_mode inline --width 400 --height 300

        The values are in pixels for the width and height. A short cut:

            In [9]: %matlab_plot_mode inline -w 400 -h 300 -r 150

        also works. The width and height only applies to inline figures.
        You should use the usual Matlab commands to resize windowed figures.
        """
        args = magic_arguments.parse_argstring(self.matlab_plot_mode, line)
        plot_type = args.plot_type if args.plot_type else self.plot_type
        output = args.output if args.output else self.output
        if args.plot_type and args.plot_type == 'inline' and args.output == None:
            output = 'inline'
        self.output = output
        if plot_type == 'inline':
            self.plot_type = plot_type
            if args.width: self.width = args.width
            if args.height: self.height = args.height
        elif plot_type == 'windowed':
            self.plot_type = plot_type
        else:
            raise RuntimeError(f'Unknown plot type {plot_type}')
        if self.m is None:
            try:
                from . import Matlab
                self.m = Matlab()._interface
            except RuntimeError:
                return
        if plot_type == 'inline':
            self.m.call('set', 0, 'defaultfigurevisible', 'off', nargout=0)
            self.m.call('set', 0, 'defaultfigurepaperpositionmode', 'manual', nargout=0)
        elif plot_type == 'windowed':
            self.m.call('set', 0, 'defaultfigurevisible', 'on', nargout=0)
            self.m.call('set', 0, 'defaultfigurepaperpositionmode', 'auto', nargout=0)
        else:
            raise RuntimeError(f'Unknown plot type {plot_type}')

    @line_magic
    @magic_arguments.magic_arguments()
    @magic_arguments.argument('-w', '--width', type=int, help="Default figure width in pixels [def: 400]")
    @magic_arguments.argument('-h', '--height', type=int, help="Default figure height in pixels [def: 300]")
    def matlab_fig(self, line):
        """Defines size of the next inline Matlab figure to be plotted

        Use this magic function to define the figure size of the next figure
        (and only that figure) without changing the default figure size.

        Examples
        --------
        The size is specified as options, any which is not defined here will use the default values
        These values are reset after the figure is plotted (default: width=400, height=300)

            In [1]: %matlab_fig -w 800 -h 200
                    m.plot(-pi:0.01:pi, sin(-pi:0.01:pi), '-')

            In [2]: m.plot(-pi:0.01:pi, cos(-pi:0.01:pi), '-')

        The sine graph in the first cell will be 800x200, whilst the cosine graph is 400x300.
        """
        args = magic_arguments.parse_argstring(self.matlab_fig, line)
        width = args.width if args.width else self.width
        height = args.height if args.height else self.height
        self.next_pars = {'width':width, 'height':height}
