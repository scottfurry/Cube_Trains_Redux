#include <iostream>
#include <math.h>

#include "asserts.hpp"
#include "color_utils.hpp"
#include "foreach.hpp"
#include "formatter.hpp"
#include "formula.hpp"
#include "level.hpp"
#include "raster.hpp"
#include "water.hpp"
#include "wml_node.hpp"
#include "wml_utils.hpp"
#include "color_utils.hpp"

namespace {
	const int WaterZorder = 3;
}

water::water()
  : zorder_(WaterZorder)
{}

water::water(wml::const_node_ptr water_node) :
  zorder_(wml::get_int(water_node, "zorder", WaterZorder)),
  current_x_formula_(game_logic::formula::create_optional_formula(water_node->attr("current_x_formula"))),
  current_y_formula_(game_logic::formula::create_optional_formula(water_node->attr("current_y_formula")))
{
	FOREACH_WML_CHILD(area_node, water_node, "area") {
		const rect r(area_node->attr("rect"));
		areas_.push_back(area(r));
	}
}

wml::node_ptr water::write() const
{
	wml::node_ptr result(new wml::node("water"));
	result->set_attr("zorder", formatter() << zorder_);
	foreach(const area& a, areas_) {
		wml::node_ptr node(new wml::node("area"));
		node->set_attr("rect", a.rect_.to_string());
		result->add_child(node);
	}

	return result;
}

void water::add_rect(const rect& r)
{
	areas_.push_back(area(r));
}

void water::delete_rect(const rect& r)
{
	for(std::vector<area>::iterator i = areas_.begin(); i != areas_.end(); ) {
		if(rects_intersect(r, i->rect_)) {
			i = areas_.erase(i);
		} else {
			++i;
		}
	}
}

void water::begin_drawing()
{
	foreach(area& a, areas_) {
		graphics::add_raster_distortion(&a.distortion_);
	}
}

void water::end_drawing() const
{
	foreach(const area& a, areas_) {
		graphics::remove_raster_distortion(&a.distortion_);
	}
}

/*
void water::set_surface_detection_rects(int zorder)
{
	const int offset = get_offset(zorder);
	foreach(area& a, areas_) {
		//detect drawing at the surface of the water.
		a.draw_detection_buf_.resize(a.rect_.w());
		memset(&a.draw_detection_buf_[0], 0, a.draw_detection_buf_.size());
		graphics::set_draw_detection_rect(rect(a.rect_.x(), a.rect_.y() + offset, a.rect_.w(), 1), &a.draw_detection_buf_[0]);
	}
}
*/

bool water::draw(int x, int y, int w, int h) const
{
	glShadeModel(GL_SMOOTH);

	bool result = false;
	foreach(const area& a, areas_) {
		if(draw_area(a, x, y, w, h)) {
			result = true;
		}
	}

	end_drawing();
	glShadeModel(GL_FLAT);

	return result;
}

void water::add_wave(const point& p, double xvelocity, double height, double length, double delta_height, double delta_length)
{
	foreach(area& a, areas_) {
		if(point_in_rect(p, a.rect_)) {
			std::pair<int, int> bounds(a.rect_.x(), a.rect_.x2());
			for(int n = 0; n != a.surface_segments_.size(); ++n) {
				if(p.x >= a.surface_segments_[n].first && p.x <= a.surface_segments_[n].second) {
					bounds = a.surface_segments_[n];
					break;
				}
			}
			wave wv = { p.x, xvelocity, height, length, delta_height, delta_length, bounds.first, bounds.second };
			a.waves_.push_back(wv);
			return;
		}
	}
}

bool water::draw_area(const water::area& a, int x, int y, int w, int h) const
{
	const graphics::color waterline_color(250, 240, 205, 255);
	const graphics::color shallowwater_color(0, 51, 61, 140);
	const graphics::color deepwater_color(0, 51, 61, 153);
	const SDL_Rect waterline_rect = {a.rect_.x(), a.rect_.y(), a.rect_.w(), 2};
	const SDL_Rect underwater_rect = {a.rect_.x(), a.rect_.y(), a.rect_.w(), a.rect_.h()};

	glEnable(GL_LINE_SMOOTH);
	#ifdef GL_POLYGON_SMOOTH
	glEnable(GL_POLYGON_SMOOTH);
	#endif

	bool draw_with_waves = false;
	if(a.waves_.empty() == false) {

		std::vector<GLfloat> heights(w);
		foreach(const wave& wv, a.waves_) {
			int begin_x = std::max<int>(wv.xpos - wv.length, x);
			int end_x = std::min<int>(wv.xpos + wv.length + 1, x + w);
			for(int xpos = begin_x; xpos < end_x; ++xpos) {
				const int distance = abs(wv.xpos - xpos);
				const double proportion = GLfloat(distance)/GLfloat(wv.length);

				const int index = xpos - x;
				ASSERT_INDEX_INTO_VECTOR(index, heights);
				heights[index] += wv.height*cos(proportion*3.14*2.0)*(1.0 - proportion);

				//see if the wave is close enough to the edge to reflect some of its force off it
				const int distance_wrap_left = (wv.xpos - wv.left_bound) + (xpos - wv.left_bound);
				if(distance_wrap_left < wv.length) {
					const double proportion = GLfloat(distance_wrap_left)/GLfloat(wv.length);
					heights[index] += wv.height*cos(proportion*3.14*2.0)*(1.0 - proportion);
				}

				const int distance_wrap_right = (wv.right_bound - wv.xpos) + (wv.right_bound - xpos);
				if(distance_wrap_right < wv.length) {
					const double proportion = GLfloat(distance_wrap_right)/GLfloat(wv.length);
					heights[index] += wv.height*cos(proportion*3.14*2.0)*(1.0 - proportion);
				}
			}
		}

		const int begin_x = std::max(x, a.rect_.x());
		const int end_x = std::min(x + w, a.rect_.x2());
		if(false && end_x > begin_x+1) {
			draw_with_waves = true;
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

			glColor4ub(0, 51, 61, 153);
			std::vector<GLfloat>& varray = graphics::global_vertex_array();
			varray.clear();
			for(int xpos = begin_x; xpos != end_x; ++xpos) {
				const int index = xpos - x;
				ASSERT_INDEX_INTO_VECTOR(index, heights);
				const GLfloat ypos = a.rect_.y() - heights[index];

				varray.push_back(xpos); varray.push_back(underwater_rect.y + 100);
				varray.push_back(xpos); varray.push_back(ypos);
			}
			glVertexPointer(2, GL_FLOAT, 0, &varray.front());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, varray.size()/2);

			varray.clear();
			varray.push_back(begin_x);
			varray.push_back(underwater_rect.y + 100);
			varray.push_back(end_x);
			varray.push_back(underwater_rect.y + 100);
			varray.push_back(begin_x);
			varray.push_back(underwater_rect.y + underwater_rect.h);
			varray.push_back(end_x);
			varray.push_back(underwater_rect.y + underwater_rect.h);
			glVertexPointer(2, GL_FLOAT, 0, &varray.front());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, varray.size()/2);

			glLineWidth(2.0);
			varray.clear();
			glColor4f(1.0, 1.0, 1.0, 1.0);
			for(int xpos = begin_x; xpos != end_x; ++xpos) {
				const int index = xpos - x;
				ASSERT_INDEX_INTO_VECTOR(index, heights);
				const GLfloat ypos = a.rect_.y() - heights[index];
				varray.push_back(xpos);
				varray.push_back(ypos);
			}
			glVertexPointer(2, GL_FLOAT, 0, &varray.front());
			glDrawArrays(GL_LINE_STRIP, 0, varray.size()/2);

			//draw a second line, in a different color, just below the first
			glColor4f(0.0, 0.9, 0.75, 0.5);
			varray.clear();
			for(int xpos = begin_x; xpos != end_x; ++xpos) {
				const int index = xpos - x;
				ASSERT_INDEX_INTO_VECTOR(index, heights);
				const GLfloat ypos = a.rect_.y() - heights[index] +2;
				varray.push_back(xpos);
				varray.push_back(ypos);
			}
			glVertexPointer(2, GL_FLOAT, 0, &varray.front());
			glDrawArrays(GL_LINE_STRIP, 0, varray.size()/2);
		}
	}

	if(draw_with_waves == false) {
		
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
		GLfloat vertices[] = {
			waterline_rect.x, waterline_rect.y, //shallow water colored
			waterline_rect.x + waterline_rect.w, waterline_rect.y,
			
			waterline_rect.x, waterline_rect.y + 100, //deep water colored
			waterline_rect.x + waterline_rect.w, waterline_rect.y + 100,
			waterline_rect.x, underwater_rect.y + underwater_rect.h,
			waterline_rect.x + waterline_rect.w, underwater_rect.y + underwater_rect.h
		};
		
		glColor4ub(70, 0, 00, 50);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices)/sizeof(GLfloat)/2);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_LINE_SMOOTH);

		glLineWidth(2.0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		GLfloat varray[] = {
			waterline_rect.x, waterline_rect.y,
			waterline_rect.x + waterline_rect.w, waterline_rect.y
		};
		glVertexPointer(2, GL_FLOAT, 0, varray);
		glDrawArrays(GL_LINES, 0, 2);
		
		//draw a second line, in a different color, just below the first
		glColor4f(0.0, 0.9, 0.75, 0.5);
		GLfloat varray2[] = {
			waterline_rect.x, waterline_rect.y+2,
			waterline_rect.x + waterline_rect.w, waterline_rect.y+2
		};
		glVertexPointer(2, GL_FLOAT, 0, varray2);
		glDrawArrays(GL_LINES, 0, 2);
	
	}

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_LINE_SMOOTH);
	#ifdef GL_POLYGON_SMOOTH
	glDisable(GL_POLYGON_SMOOTH);
	#endif

	return true;
}

namespace {
bool wave_dead(const water::wave& w) {
	return w.height <= 0.5 || w.length <= 0;
}
}

void water::process(const level& lvl)
{
	foreach(area& a, areas_) {
		init_area_surface_segments(lvl, a);

		a.distortion_ = graphics::water_distortion(lvl.cycle(), a.rect_);
		foreach(wave& w, a.waves_) {
			w.process();

			//if the wave has hit the edge, then turn it around.
			if(w.xpos < w.left_bound && w.xvelocity < 0) {
				w.xvelocity *= -1.0;
			}

			if(w.xpos > w.right_bound && w.xvelocity > 0) {
				w.xvelocity *= -1.0;
			}
		}

		a.waves_.erase(std::remove_if(a.waves_.begin(), a.waves_.end(), wave_dead), a.waves_.end());
	}
}

void water::wave::process() {
	xpos += xvelocity;
	height *= 0.996;
	length += delta_length;
}

void water::get_current(const entity& e, int* velocity_x, int* velocity_y) const
{
	if(velocity_x && current_x_formula_) {
		*velocity_x += current_x_formula_->execute(e).as_int();
	}

	if(velocity_y && current_y_formula_) {
		*velocity_y += current_y_formula_->execute(e).as_int();
	}
}

bool water::is_underwater(const rect& r, rect* result_water_area) const
{
	const point p((r.x() + r.x2())/2, (r.y() + r.y2())/2);
	foreach(const area& a, areas_) {
		if(point_in_rect(p, a.rect_)) {
			if(result_water_area) {
				*result_water_area = a.rect_;
			}
			return true;
		}
	}

	return false;
}

void water::init_area_surface_segments(const level& lvl, water::area& a)
{
	if(a.surface_segments_.empty() == false) {
		return;
	}

	bool prev_solid = true;
	int begin_segment = 0;
	for(int x = a.rect_.x(); x != a.rect_.x2(); ++x) {
		const bool solid = lvl.solid(x, a.rect_.y()) || x == a.rect_.x2()-1;
		if(solid && !prev_solid) {
			a.surface_segments_.push_back(std::make_pair(begin_segment, x));
		} else if(!solid && prev_solid) {
			begin_segment = x;
		}

		prev_solid = solid;
	}

	if(a.surface_segments_.empty()) {
		a.surface_segments_.push_back(std::make_pair(0,0));
	}
}
