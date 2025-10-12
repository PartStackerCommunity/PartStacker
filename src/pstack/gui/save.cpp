#include "pstack/gui/save.hpp"
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <cstddef>
#include <expected>
#include <string_view>
#include <string>
#include <ranges>
#include <utility>
#include <vector>

namespace pstack::gui {
namespace {

struct save_part : calc::part_base {
    bool in_parts_list;
};

struct save_sinterbox_parameters : calc::sinterbox_settings, calc::sinterbox_bounding {};

using save_result = calc::stack_result_base<std::size_t, save_sinterbox_parameters>;

using internal_save_state = basic_save_state<save_part, save_result>;

} // namespace
} // namespace pstack::gui

JSONCONS_N_MEMBER_TRAITS(pstack::gui::preferences, 0, // Nothing is required
    invert_scroll, extra_parts, show_bounding_box, load_environment_popup
);
JSONCONS_N_MEMBER_TRAITS(pstack::calc::stack_settings, 0, // Nothing is required
    resolution, x_min, x_max, y_min, y_max, z_min, z_max
);
JSONCONS_N_MEMBER_TRAITS(pstack::calc::sinterbox_settings, 0, // Nothing is required
    clearance, thickness, width, spacing
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_part,
    mesh_file, quantity, mirrored, min_hole, rotation_index, rotate_min_box, in_parts_list
);

JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::point3<float>,
    x, y, z
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::vector3<float>,
    x, y, z
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::geo::matrix3<float>,
    xx, xy, xz, yx, yy, yz, zx, zy, zz
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_result::piece,
    part, rotation, translation
);
JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::save_sinterbox_parameters,
    clearance, thickness, width, spacing, min, max
);
JSONCONS_N_MEMBER_TRAITS(pstack::gui::save_result, 1,
    pieces,     // mandatory
    sinterbox   // optional
);

JSONCONS_ALL_MEMBER_TRAITS(pstack::gui::internal_save_state,
    preferences, stack, sinterbox, parts, results
);

namespace {

const std::string_view the_schema = R"the_schema({
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "description": "A representation of the state of the PartStacker project",
    "type": "object",
    "$defs": {
        "unsigned_int": {
            "type": "integer",
            "minimum": 0
        },
        "point3": {
            "type": "object",
            "properties": {
                "x": { "type": "number" },
                "y": { "type": "number" },
                "z": { "type": "number" }
            },
            "required": ["x", "y", "z"]
        },
        "matrix3": {
            "type": "object",
            "properties": {
                "xx": { "type": "number" },
                "xy": { "type": "number" },
                "xz": { "type": "number" },
                "yx": { "type": "number" },
                "yy": { "type": "number" },
                "yz": { "type": "number" },
                "zx": { "type": "number" },
                "zy": { "type": "number" },
                "zz": { "type": "number" }
            },
            "required": ["xx", "xy", "xz", "yx", "yy", "yz", "zx", "zy", "zz"]
        }
    },
    "properties": {
        "preferences": {
            "type": "object",
            "properties": {
                "invert_scroll": { "type": "boolean" },
                "extra_parts": { "type": "boolean" },
                "show_bounding_box": { "type": "boolean" },
                "load_environment_popup": { "type": "boolean" }
            }
        },
        "stack": {
            "type": "object",
            "properties": {
                "resolution": { "type": "number" },
                "x_min": { "$ref": "#/$defs/unsigned_int" },
                "x_max": { "$ref": "#/$defs/unsigned_int" },
                "y_min": { "$ref": "#/$defs/unsigned_int" },
                "y_max": { "$ref": "#/$defs/unsigned_int" },
                "z_min": { "$ref": "#/$defs/unsigned_int" },
                "z_max": { "$ref": "#/$defs/unsigned_int" }
            }
        },
        "sinterbox": {
            "type": "object",
            "properties": {
                "clearance": { "type": "number" },
                "thickness": { "type": "number" },
                "width": { "type": "number" },
                "spacing": { "type": "number" }
            }
        },
        "parts": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "mesh_file": { "type": "string" },
                    "quantity": { "$ref": "#/$defs/unsigned_int" },
                    "mirrored": { "type": "boolean" },
                    "min_hole": { "$ref": "#/$defs/unsigned_int" },
                    "rotation_index": { "type": "integer", "minimum": 0, "maximum": 2 },
                    "rotate_min_box": { "type": "boolean" },
                    "in_parts_list": { "type": "boolean" }
                },
                "required": ["mesh_file", "quantity", "mirrored", "min_hole", "rotation_index", "rotate_min_box", "in_parts_list"]
            }
        },
        "results": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "pieces": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "part": { "$ref": "#/$defs/unsigned_int" },
                                "rotation": { "$ref": "#/$defs/matrix3" },
                                "translation": { "$ref": "#/$defs/point3" }
                            },
                            "required": ["rotation", "translation", "part"]
                        }
                    },
                    "sinterbox": {
                        "type": "object",
                        "properties": {
                            "clearance": { "type": "number" },
                            "thickness": { "type": "number" },
                            "width": { "type": "number" },
                            "spacing": { "type": "number" },
                            "min": { "$ref": "#/$defs/point3" },
                            "max": { "$ref": "#/$defs/point3" }
                        },
                        "required": ["clearance", "thickness", "width", "spacing", "min", "max"]
                    }
                },
                "required": ["pieces"]
            }
        }
    },
    "required": ["preferences", "stack", "sinterbox", "parts", "results"]
})the_schema";

} // namespace

namespace pstack::gui {

namespace {

in_save_state from_internal(const internal_save_state& state) {
	in_save_state s{};
	s.preferences = state.preferences;
	s.stack = state.stack;
	s.sinterbox = state.sinterbox;

	for (const save_part& part : state.parts) {
		auto& list = part.in_parts_list ? s.parts : s.extra_parts;
		calc::part p = calc::initialize_part(static_cast<const calc::part_base&>(part));
		list.push_back(std::make_shared<calc::part>(std::move(p)));
	}

	for (const save_result& result : state.results) {
		auto& r = s.results.emplace_back();
		if (result.sinterbox.has_value()) {
			r.sinterbox.emplace();
			r.sinterbox->settings = static_cast<const calc::sinterbox_settings&>(*result.sinterbox);
			r.sinterbox->bounding = static_cast<const calc::sinterbox_bounding&>(*result.sinterbox);
		}
		for (const auto& piece : result.pieces) {
			auto& p = r.pieces.emplace_back();
			p.rotation = piece.rotation;
			p.translation = piece.translation;
			if (piece.part < s.parts.size()) {
				p.part = s.parts[piece.part];
			} else {
				p.part = s.extra_parts[piece.part - s.parts.size()];
			}
		}
	}

    return s;
};

internal_save_state to_internal(const out_save_state& state) {
    std::vector all_parts = state.parts; // Copy

    std::vector<save_result> results{};
    for (const auto& in_result : state.results) {
        auto& out_result = results.emplace_back();
        if (in_result.sinterbox.has_value()) {
            out_result.sinterbox.emplace();
            static_cast<calc::sinterbox_settings&>(*out_result.sinterbox) = in_result.sinterbox->settings;
            static_cast<calc::sinterbox_bounding&>(*out_result.sinterbox) = in_result.sinterbox->bounding;
        }
        for (const auto& in_piece : in_result.pieces) {
            auto& out_piece = out_result.pieces.emplace_back();
            out_piece.rotation = in_piece.rotation;
            out_piece.translation = in_piece.translation;
            auto it = std::ranges::find(all_parts, in_piece.part);
            if (it != all_parts.end()) {
                out_piece.part = std::ranges::distance(all_parts.begin(), it);
            } else {
                out_piece.part = all_parts.size();
                all_parts.push_back(in_piece.part);
            }
        }
    }

    std::vector<save_part> parts{};
    for (std::size_t i = 0; i != all_parts.size(); ++i) {
        const calc::part_base& part = *(all_parts[i]);
        parts.emplace_back(part);
        parts.back().in_parts_list = (i < state.parts.size());
    }

    return internal_save_state{
        .preferences = state.preferences,
        .stack = state.stack,
        .sinterbox = state.sinterbox,
        .parts = std::move(parts),
        .results = std::move(results),
    };
};

} // namespace

std::expected<in_save_state, std::string> save_state_from_json(std::string_view str) {
    try {

        const jsoncons::json j = jsoncons::json::parse(str);

        static const auto schema = []{
            auto schema = jsoncons::json::parse(the_schema);
            return jsoncons::jsonschema::make_json_schema(std::move(schema));
        }();

        std::vector<std::string> errors;
        const auto schema_reporter = [&](const jsoncons::jsonschema::validation_message& message) {
            errors.push_back(std::format("{}: {}", message.instance_location().string(), message.message()));
            return jsoncons::jsonschema::walk_result::advance;
        };
        schema.validate(j, schema_reporter);
        if (not errors.empty()) {
            std::string full_error = "File does not conform to schema:";
            for (std::string& error : errors) {
                full_error += "\n    " + std::move(error);
            }
            return std::unexpected(std::move(full_error));
        }

        return from_internal(j.as<internal_save_state>());

    } catch (const jsoncons::ser_error& e) {
        return std::unexpected(std::string("Failed to parse JSON with ser_error: ") + e.what());
    } catch (const jsoncons::json_exception& e) {
        return std::unexpected(std::string("Failed to parse JSON with json_exception: ") + e.what());
    } catch (const std::exception& e) {
        return std::unexpected(std::string("Failed to parse JSON with std::exception: ") + e.what());
    } catch (...) {
        return std::unexpected("Failed to parse JSON with unknown error");
    }
}

std::string save_state_to_json(const out_save_state& state) {
    return jsoncons::json(to_internal(state)).to_string();
}

} // namespace pstack::gui
