#ifndef CUSTOM_OBJECT_TYPE_HPP_INCLUDED
#define CUSTOM_OBJECT_TYPE_HPP_INCLUDED

#include <map>
#include <string>

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

#include "formula.hpp"
#include "formula_function.hpp"
#include "frame.hpp"
#include "particle_system.hpp"
#include "solid_map_fwd.hpp"
#include "variant.hpp"
#include "wml_node.hpp"

class custom_object_type;

typedef boost::shared_ptr<custom_object_type> custom_object_type_ptr;
typedef boost::shared_ptr<const custom_object_type> const_custom_object_type_ptr;

namespace wml {
class modifier;
typedef boost::shared_ptr<const modifier> const_modifier_ptr;
}

class custom_object_type
{
public:
	static const_custom_object_type_ptr get(const std::string& id);

	typedef std::map<std::string, game_logic::const_formula_ptr> event_handler_map;

	static void init_event_handlers(wml::const_node_ptr node,
	                                event_handler_map& handlers,
									game_logic::function_symbol_table* symbols=0);

	explicit custom_object_type(wml::const_node_ptr node);
	~custom_object_type();

	const std::string& id() const { return id_; }
	int hitpoints() const { return hitpoints_; }

	int timer_frequency() const { return timer_frequency_; }

	const frame& default_frame() const;
	const frame& get_frame(const std::string& key) const;

	const game_logic::const_formula_ptr& next_animation_formula() const { return next_animation_formula_; }

	game_logic::const_formula_ptr get_event_handler(const std::string& event) const;

	int zorder() const { return zorder_; }
	bool dies_on_inactive() const { return dies_on_inactive_;}
	bool always_active() const { return always_active_;}
	bool body_harmful() const { return body_harmful_; }
	bool body_passthrough() const { return body_passthrough_; }
	bool ignore_collide() const { return ignore_collide_; }

	bool object_level_collisions() const { return object_level_collisions_; }

	int springiness() const { return springiness_; }
	int surface_friction() const { return surface_friction_; }
	int surface_traction() const { return surface_traction_; }
	int mass() const { return mass_; }

	//amount of friction we experience.
	int friction() const { return friction_; }
	int traction() const { return traction_; }
	int traction_in_air() const { return traction_in_air_; }

	bool on_players_side() const { return on_players_side_; }
	bool respawns() const { return respawns_; }

	bool affected_by_currents() const { return affected_by_currents_; }

	wml::const_node_ptr get_child(const std::string& key) const {
		if(children_.count(key)) {
			return children_.find(key)->second;
		}

		return wml::const_node_ptr();
	}

	const_particle_system_factory_ptr get_particle_system_factory(const std::string& id) const;

	bool is_vehicle() const { return is_vehicle_; }

	int passenger_x() const { return passenger_x_; }
	int passenger_y() const { return passenger_y_; }

	int feet_width() const { return feet_width_; }

	int teleport_offset_x() const { return teleport_offset_x_; }
	int teleport_offset_y() const { return teleport_offset_y_; }

	bool use_image_for_collisions() const { return use_image_for_collisions_; }
	bool has_feet() const { return has_feet_; }

	const std::map<std::string, variant>& variables() const { return variables_; }
	const std::vector<std::string>& tags() const { return tags_; }

	const std::map<std::string, game_logic::const_formula_ptr>& properties() const { return properties_; }

	game_logic::function_symbol_table* function_symbols() const;

	const_solid_info_ptr solid() const { return solid_; }
	const_solid_info_ptr platform() const { return platform_; }

	//true if the object can ever be solid or standable
	bool has_solid() const { return has_solid_; }

	unsigned int solid_dimensions() const { return solid_dimensions_; }

	const_custom_object_type_ptr get_variation(const std::vector<std::string>& variations) const;

private:
	std::string id_;
	int hitpoints_;

	int timer_frequency_;

	typedef std::map<std::string, std::vector<boost::shared_ptr<frame> > > frame_map;
	frame_map frames_;

	boost::shared_ptr<frame> default_frame_;

	game_logic::const_formula_ptr next_animation_formula_;

	event_handler_map event_handlers_;
	boost::shared_ptr<game_logic::function_symbol_table> object_functions_;

	int zorder_;

	bool dies_on_inactive_;
	bool always_active_;
	bool body_harmful_;
	bool body_passthrough_;
	bool ignore_collide_;
	bool object_level_collisions_;

	int springiness_;
	int surface_friction_;
	int surface_traction_;
	int friction_, traction_, traction_in_air_;
	int mass_;

	bool on_players_side_;
	bool respawns_;

	bool affected_by_currents_;

	std::map<std::string, wml::const_node_ptr> children_;

	wml::const_node_ptr node_;

	std::map<std::string, const_particle_system_factory_ptr> particle_factories_;

	bool is_vehicle_;
	int passenger_x_, passenger_y_;
	int feet_width_;

	bool use_image_for_collisions_;

	bool has_feet_;

	std::map<std::string, variant> variables_;
	std::vector<std::string> tags_;

	std::map<std::string, game_logic::const_formula_ptr> properties_;

	int teleport_offset_x_, teleport_offset_y_;

	const_solid_info_ptr solid_, platform_;

	//variable which is true if the object is ever solid or standable
	bool has_solid_;

	unsigned int solid_dimensions_;

	std::map<std::string, wml::const_modifier_ptr> variations_;
	mutable std::map<std::vector<std::string>, const_custom_object_type_ptr> variations_cache_;
};

#endif
