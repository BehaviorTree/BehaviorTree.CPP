# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

set(tests
        detail/buffer_builder.cpp
        detail/integer_sequence.cpp
        detail/invoke.cpp
        detail/lazy_init.cpp
        detail/nttp_string.cpp
        detail/stateless_lambda.cpp
        detail/std.cpp
        detail/string_view.cpp
        detail/tuple.cpp
        detail/type_name.cpp

        action/base.cpp
        action/match.cpp
        action/parse.cpp
        action/parse_as_tree.cpp
        action/scan.cpp
        action/trace.cpp
        action/validate.cpp

        callback/adapter.cpp
        callback/aggregate.cpp
        callback/base.cpp
        callback/bind.cpp
        callback/bit_cast.cpp
        callback/composition.cpp
        callback/constant.cpp
        callback/container.cpp
        callback/fold.cpp
        callback/forward.cpp
        callback/integer.cpp
        callback/noop.cpp
        callback/object.cpp
        callback/string.cpp

        dsl/verify.hpp
        dsl/any.cpp
        dsl/argv_input.cpp
        dsl/ascii.cpp
        dsl/base.cpp
        dsl/bits.cpp
        dsl/bom.cpp
        dsl/brackets.cpp
        dsl/branch.cpp
        dsl/byte.cpp
        dsl/capture.cpp
        dsl/case_folding.cpp
        dsl/char_class.cpp
        dsl/choice.cpp
        dsl/code_point.cpp
        dsl/combination.cpp
        dsl/context_counter.cpp
        dsl/context_flag.cpp
        dsl/context_identifier.cpp
        dsl/delimited.cpp
        dsl/digit.cpp
        dsl/eof.cpp
        dsl/error.cpp
        dsl/expression.cpp
        dsl/follow.cpp
        dsl/list.cpp
        dsl/literal.cpp
        dsl/lookahead.cpp
        dsl/loop.cpp
        dsl/identifier.cpp
        dsl/if.cpp
        dsl/integer.cpp
        dsl/member.cpp
        dsl/newline.cpp
        dsl/operator.cpp
        dsl/option.cpp
        dsl/parse_as.cpp
        dsl/peek.cpp
        dsl/position.cpp
        dsl/production.cpp
        dsl/punctuator.cpp
        dsl/recover.cpp
        dsl/repeat.cpp
        dsl/return.cpp
        dsl/scan.cpp
        dsl/sequence.cpp
        dsl/separator.cpp
        dsl/sign.cpp
        dsl/symbol.cpp
        dsl/terminator.cpp
        dsl/trace.cpp
        dsl/token.cpp
        dsl/times.cpp
        dsl/unicode.cpp
        dsl/until.cpp
        dsl/whitespace.cpp

        input/argv_input.cpp
        input/base.cpp
        input/buffer.cpp
        input/file.cpp
        input/lexeme_input.cpp
        input/range_input.cpp
        input/string_input.cpp

        callback.cpp
        code_point.cpp
        encoding.cpp
        error.cpp
        grammar.cpp
        input_location.cpp
        lexeme.cpp
        parse_tree.cpp
        token.cpp
        visualize.cpp
    )

add_executable(lexy_test ${tests})
target_link_libraries(lexy_test PRIVATE lexy_test_base)

