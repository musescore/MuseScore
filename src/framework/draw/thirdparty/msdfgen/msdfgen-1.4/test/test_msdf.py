import argparse
import os
import re
import sys
import codecs
import tempfile
from subprocess import Popen, PIPE, call


def println(msg):
    sys.stdout.write(msg)
    sys.stdout.write('\n')
    sys.stdout.flush()
    return msg


class Runner:
    pass_count = 0
    fail_count = 0
    stop_on_fail = False
    results = []

    def __init__(self):
        pass

    def add_pass(self, input_path, msg, out_path=None, render_path=None, diff_path=None):
        return self.add_result(True, input_path, msg, out_path, render_path, diff_path)

    def add_fail(self, input_path, msg, out_path=None, render_path=None, diff_path=None):
        return self.add_result(False, input_path, msg, out_path, render_path, diff_path)

    def add_result(self, passed, input_path, msg, out_path=None, render_path=None, diff_path=None):
        if passed:
            self.pass_count += 1
            println("PASS \"%s\" %s" % (input_path, msg))
        else:
            self.fail_count += 1
            println("FAIL \"%s\" %s" % (input_path, msg))
            if self.stop_on_fail:
                sys.exit(1)

        self.results.append({
            'pass': passed,
            'in': input_path,
            'out': out_path,
            'render': render_path,
            'diff': diff_path})

        return passed

    def get_outputs(self, key):
        ret = []
        for e in self.results:
            if key in e:
                ret.append(e[key])
        return ret


def check_imagemagick():
    p = Popen(["compare -version"],
              stdin=PIPE, stdout=PIPE, stderr=PIPE, shell=True)
    output, err = p.communicate("")
    if p.returncode != 0:
        println("Cannot find ImageMagick <http://www.imagemagick.org> on your PATH. It is required to run tests.")
        sys.exit(1)


def compare(ref_path, render_path, diff_path, total_pixels, args):
    # Use imagemagick to rasterize the reference image and compare it.
    # We use a very fuzzy comparison to try and account for differences in aliasing introduced around edges of
    # an image, which comes from both the down/up rezzing introduced by going through the SDF process, but also
    # minor differences with IM's SVG rasterizer. What we're trying to catch is full on pixel mismatches that
    # indicate a shape was mis-handled (curve shooting off to the edge, fill rule violation, etc).

    p = Popen(["compare -fuzz %f%% -metric AE \"%s\" \"%s\" \"%s\"" % (args.fuzz, ref_path, render_path, diff_path)],
              stdin=PIPE, stdout=PIPE, stderr=PIPE, shell=True)
    output, err = p.communicate("")
    passed = False
    if p.returncode == 1:
        # Ran successfully, but they are different. Let's parse the output and check the actual error metric.
        match = re.match(r'^(\d+)$', err)
        passed = False
        if match:
            try:
                pixels = int(match.group(1))
                e = float(pixels) / total_pixels * 100.0
                if e > args.fail_threshold:
                    msg = "Error = %.3f %% (%d)" % (e, pixels)
                else:
                    passed = True
                    msg = "OK = %.3f %% (%d)" % (e, pixels)
            except ValueError:
                msg = "(Unknown) Error metric = %s" % err
        else:
            msg = "(Unknown) Error metric = %s" % err
    elif p.returncode != 0:
        msg = "Error comparing %s to %s [%d]: %s" % (ref_path, render_path, p.returncode, err)
    else:
        passed = True
        msg = "Identical"

    return (passed, msg)


def test_svg(path, args, runner):
    (root, ext) = os.path.splitext(path)

    mode = args.mode
    sz = args.sdf_size
    rsz = str(args.render_size)

    sdf_path = "%s.%s.png" % (root, mode)
    render_path = "%s.%s-render.png" % (root, mode)
    diff_path = "%s.%s-diff.png" % (root, mode)

    try:
        result = call([args.exe, mode, '-svg', path, '-o', sdf_path,
                       # '-angle', '4',
                       # '-pxrange', '2',
                       # '-range', '8',
                       # '-tolerance', '0.01',
                       # '-legacy', str(args.legacy),
                       '-scale', str(sz / float(args.render_size)), '-size', str(sz), str(sz),
                       # '-exportshape', 'shape.txt',
                       '-testrender', render_path, rsz, rsz
                       ], shell=False)
        if result != 0:
            return runner.add_fail(path, "Unable to render %s" % mode)
    except OSError as er:
        return runner.add_fail(path, "Error running %s: %s" % (args.exe, er))

    total_pixels = args.render_size * args.render_size
    (passed, msg) = compare(path, render_path, diff_path, total_pixels, args)

    return runner.add_result(passed, path, msg, sdf_path, render_path, diff_path)


def test_shapedesc(path, args, runner):

    # Allow either *.shapedesc.txt or *.shapedesc
    (root, ext) = os.path.splitext(path)
    if ext == ".txt":
        (root, ext) = os.path.splitext(root)
    
    mode = args.mode
    sz = args.sdf_size
    
    # Ignore specified render size, and match the reference image provided.

    sdf_path = "%s.%s.png" % (root, mode)
    ref_path = "%s.%s-ref.png" % (root, mode)
    render_path = "%s.%s-render.png" % (root, mode)
    diff_path = "%s.%s-diff.png" % (root, mode)
    
    p = Popen(["identify \"%s\"" % ref_path], stdin=PIPE, stdout=PIPE, shell=True)
    output, err = p.communicate("")
    if p.returncode != 0:
        return runner.add_result(False, path, "Error identifying reference image %s: %s" % (ref_path, err))

    match = re.match(r'.+\s\S+\s(\d+)x(\d+)\s', output)
    if not match:
        return runner.add_result(False, path, "Could not determine size of reference image")

    rw = match.group(1)
    rh = match.group(2)
    
    try:
        result = call([args.exe, mode, '-shapedesc', path, '-o', sdf_path,
                        '-autoframe',
                        '-testrender', render_path, rw, rh], shell=False)
        if result != 0:
            return runner.add_fail(path, "Unable to render %s" % mode)
    except OSError as er:
        return runner.add_fail(path, "Error running %s: %s" % (args.exe, er))

    total_pixels = int(rw) * int(rh)
    (passed, msg) = compare(ref_path, render_path, diff_path, total_pixels, args)
    return runner.add_result(passed, path, msg, sdf_path, render_path, diff_path)


def main():
    parser = argparse.ArgumentParser(description="Test MSDFGEN outputs")

    parser.add_argument("--svg-dir",
                        help="Directory to scan for SVG files.")
    parser.add_argument("--svg",
                        help="SVG file to test",
                        action='append')
    parser.add_argument("--shapedesc",
                        help="Shapedesc file to test",
                        action='append')
    parser.add_argument("--mode",
                        help="Algorithm: [sdf, psdf, msdf] (default=msdf)",
                        default="msdf")
    parser.add_argument("--sdf_size",
                        help="Size for rendered (M) image.  Default = 128",
                        default=128,
                        type=int)
    parser.add_argument("--render_size",
                        help="Size for the rendered test image.  Default = 512",
                        default=512,
                        type=int)
    parser.add_argument("--exe",
                        help="Path to MSDFGEN executable",
                        default="msdfgen")
    parser.add_argument("--fail-threshold",
                        help="Percentage of pixels that are allowed to be different without flagging a failure. (Default: 0.06%%)",
                        default=0.06,  # Found experimentally (~150.0 / (512 * 512))
                        type=float)
    parser.add_argument("--fuzz",
                        help="Fuzz percentage to use when comparing",
                        default=99.5,
                        type=float)
    parser.add_argument("--legacy",
                        help="Use legacy <Version> mode algorithm",
                        default='0')
    parser.add_argument("--montage",
                        help="Generate montage image of failed results (diffs)",
                        default=False,
                        action='store_true')
    parser.add_argument("--stop-on-fail",
                        help="Stop testing after the first fail",
                        default=False,
                        action='store_true')

    args = parser.parse_args()

    if not args.svg_dir and not args.svg and not args.shapedesc:
        parser.print_help()
        sys.exit(1)

    check_imagemagick()

    runner = Runner()
    runner.stop_on_fail = args.stop_on_fail

    if args.svg_dir:
        for root, dirs, files in os.walk(args.svg_dir):
            for name in files:
                if not name.endswith(".svg"):
                    continue
                path = os.path.join(root, name)
                test_svg(path, args, runner)
    if args.svg:
        for file in args.svg:
            if file.startswith('@'):
                # Use ImageMagick style @filename.txt for loading filenames to test.
                for line in codecs.open(file[1:], 'r', 'utf-8'):
                    test_svg(line.rstrip('\n'), args, runner)
            else:
                test_svg(file, args, runner)
    
    if args.shapedesc:
        for file in args.shapedesc:
            if file.startswith('@'):
                # Use ImageMagick style @filename.txt for loading filenames to test.
                for line in codecs.open(file[1:], 'r', 'utf-8'):
                    test_shapedesc(line.rstrip('\n'), args, runner)
            else:
                test_shapedesc(file, args, runner)
    

    if args.montage:
        # We keep the diff montage at actual size (we want to see details)
        with tempfile.NamedTemporaryFile(delete=False) as temp_file:
            temp_file.write("\n".join(runner.get_outputs('diff')))
            temp_file.close()
            montage_name = "montage-%s-%s-diff.png" % (args.mode, args.legacy)
            call(["montage -geometry +1+1 @%s %s" % (temp_file.name, montage_name)], shell=True)
            os.unlink(temp_file.name)
            # print("Wrote: %s" % montage_name)

        # The others, we can let IM do some resizing because it's really just for quick glances.
        # call(["montage %s montage-%s-%s-out.png" %
        #       (" ".join(runner.get_outputs('out')), args.mode, args.legacy)], shell=True)
        # call(["montage %s montage-%s-%s-render.png" %
        #       (" ".join(runner.get_outputs('render')), args.mode, args.legacy)], shell=True)

    if runner.fail_count > 0:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == '__main__':
    main()
