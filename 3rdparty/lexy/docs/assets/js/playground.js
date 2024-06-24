const api = "https://godbolt.org/api/";
const compiler_id = "clang_trunk";
const lexy_id = { id: 'lexy', version: 'trunk' };

async function fetch_local_file(url)
{
    const response = await fetch(url);
    if (!response.ok)
        return "";
    return await response.text();
}

function json_stringify(object)
{
    // I can't use JSON.stringify directly, as the resulting JSON is base64 encoded.
    // This can't deal with Unicode code points, so I need to escape them in the JSON representation first.
    // Code taken (and fixed) from: https://stackoverflow.com/a/31652607.
    var json = JSON.stringify(object);
    json = json.replace(/[\u0080-\uFFFF]/g, function(c) {
        return "\\u" + ("0000" + c.charCodeAt(0).toString(16)).substr(-4)
    });
    return json;
}

export function list_of_productions(source)
{
    var result = [];

    const regex = /(struct|class|using) ([a-zA-Z0-9_]+)/g;
    var match = undefined;
    while (match = regex.exec(source))
        result.push(match[2]);

    return result;
}

export async function preprocess_source(target, source, production)
{
    /*
    {{ $playground_headers := resources.Get "cpp/playground_headers.single.hpp" }}
    {{ $playground_prefix  := resources.Get "cpp/playground_prefix.cpp" }}
    {{ $playground_main    := resources.Get "cpp/playground_main.cpp" }}
    {{ $godbolt_prefix     := resources.Get "cpp/godbolt_prefix.cpp" }}
    {{ $godbolt_main       := resources.Get "cpp/godbolt_main.cpp" }}
    */

    if (target == 'playground')
    {
        const header = await fetch_local_file('{{ $playground_headers.RelPermalink }}');
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`;
        const prefix = await fetch_local_file('{{ $playground_prefix.RelPermalink }}');
        const main   = await fetch_local_file('{{ $playground_main.RelPermalink }}');

        return header + '\n' + macros + '\n' + prefix + '\n' + source + '\n' + main;
    }
    else
    {
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`;
        const prefix = await fetch_local_file('{{ $godbolt_prefix.RelPermalink }}');
        const main   = await fetch_local_file('{{ $godbolt_main.RelPermalink }}');

        return macros + '\n' + prefix + source + '\n' + main;
    }
}

export async function compile_and_run(source, input, mode)
{
    var body = {};
    body.source = source;

    body.options = {};
    body.options.userArguments = "-fno-color-diagnostics -std=c++20";
    body.options.executeParameters = { args: [mode], stdin: input };
    body.options.compilerOptions = { executorRequest: true };
    body.options.filters = { execute: true };
    body.options.tools = [];
    body.options.libraries = [ lexy_id ];

    body.lang = "c++";

    const response = await fetch(`${api}/compiler/${compiler_id}/compile`, {
        method: "POST",
        headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
        body: json_stringify(body)
    });
    if (!response.ok)
        return { success: false, message: `Compiler Explorer error: ${response.status} - ${response.statusText}` };

    const result = await response.json();

    if (result.didExecute)
    {
        if (result.code == 3)
        {
            var message = result.stderr.map(x => x.text).join("\n");
            return { success: false, message: message };
        }
        else
        {
            var stdout = result.stdout.map(x => x.text).join("\n");
            var stderr = result.stderr.map(x => x.text).join("\n");
            return { success: true, stdout: stdout, stderr: stderr, code: result.code };
        }
    }
    else
    {
        var message = result.buildResult.stderr.map(x => x.text).join("\n");
        return { success: false, message: message };
    }
}

function get_godbolt_clientstate(source, input)
{
    var session = {};
    session.id = 1;
    session.language = "c++";
    session.source = source;
    session.compilers = [];

    var compiler = {};
    compiler.id = compiler_id;
    compiler.libs = [ lexy_id ];
    compiler.options = "-std=c++20";
    session.executors = [{ compiler: compiler, stdin: input, stdinVisible: true }];

    return { sessions: [session] };
}

export async function get_godbolt_permalink(source, input)
{
    const state = get_godbolt_clientstate(source, input);
    const response = await fetch(api + "shortener", {
        method: "POST",
        headers: { "Content-Type": "application/json", "Accept": "application/json" },
        body: json_stringify(state)
    });
    if (!response.ok)
        return get_godbolt_url(source, input);
    return (await response.json()).url;
}

export function get_godbolt_url(source, input)
{
    const state = get_godbolt_clientstate(source, input);
    const state_str = json_stringify(state);
    return "https://godbolt.org/clientstate/" + encodeURIComponent(btoa(state_str));
}

export async function load_example(url)
{
    const source_regex = /\/\/ INPUT:(.*)\n/;
    const source       = await fetch_local_file(url);

    const grammar = source.replace(source_regex, '');
    const input   = (source_regex.exec(source)?.[1] ?? "").replaceAll("\\n", "\n");

    return { grammar: grammar.trim(), input: input, production: "production" };
}

export async function load_godbolt_url(id)
{
    const response = await fetch(api + "shortlinkinfo/" + id);
    if (!response.ok)
        return { grammar: "", input: "", production: "" };
    const result = await response.json();

    const session = result.sessions[0];
    const source = session.source;
    const input = session.executors[0].stdin;

    const production_regex = /#define LEXY_PLAYGROUND_PRODUCTION ([a-zA-Z_0-9]+)/;
    const production = production_regex.exec(source)[1];

    const grammar_regex = /\/\/=== grammar ===\/\/([^]*)\/\/=== main function ===\/\//;
    let grammar = grammar_regex.exec(source)[1];
    grammar = grammar.trim();

    return { grammar: grammar, input: input, production: production };
}

