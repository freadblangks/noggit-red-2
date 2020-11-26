#include "ViewportGizmo.hpp"
#include "noggit/ModelInstance.h"
#include "noggit/WMOInstance.h"
#include "math/vector_3d.hpp"
#include "external/glm/glm.hpp"
#include <external/glm/gtx/matrix_decompose.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/glm/gtx/string_cast.hpp>

#include <limits>


using namespace noggit::Red::ViewportGizmo;

ViewportGizmo::ViewportGizmo(noggit::Red::ViewportGizmo::GizmoContext gizmo_context, World* world)
: _gizmo_context(gizmo_context)
, _world(world)
{}

void ViewportGizmo::handleTransformGizmo(const std::vector<selection_type>& selection
                                        , math::matrix_4x4 const& model_view
                                        , math::matrix_4x4 const& projection)
{

  if (!isUsing())
  {
    _last_pivot_scale = 1.f;
  }

  GizmoInternalMode gizmo_selection_type;

  auto model_view_trs = model_view.transposed();
  auto projection_trs = projection.transposed();

  int n_selected = selection.size();

  if (!n_selected || (n_selected == 1 & selection[0].which() == eEntry_MapChunk))
    return;

  if (n_selected == 1)
  {
    gizmo_selection_type = selection[0].which() == eEntry_Model ? GizmoInternalMode::MODEL : GizmoInternalMode::WMO;
  }
  else
  {
    gizmo_selection_type = GizmoInternalMode::MULTISELECTION;
  }

  WMOInstance* wmo_instance;
  ModelInstance* model_instance;

  ImGuizmo::SetID(_gizmo_context);

  ImGuizmo::SetDrawlist();

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetScaleGizmoAxisLock(true);
  ImGuizmo::BeginFrame();

  ImGuiIO& io = ImGui::GetIO();
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

  math::matrix_4x4 delta_matrix = math::matrix_4x4(math::matrix_4x4::unit).transposed();
  math::matrix_4x4 object_matrix = {math::matrix_4x4::unit};
  math::matrix_4x4 pivot_matrix = math::matrix_4x4(math::matrix_4x4::translation,
                                                   {_multiselection_pivot.x,
                                                    _multiselection_pivot.y,
                                                    _multiselection_pivot.z}).transposed();
  float last_pivot_scale = 1.f;

  switch (gizmo_selection_type)
  {
    case MODEL:
    {
      model_instance = boost::get<selected_model_type>(selection[0]);
      model_instance->recalcExtents();
      object_matrix = math::matrix_4x4(model_instance->transform_matrix_transposed());
      ImGuizmo::Manipulate(model_view_trs, projection_trs, _gizmo_operation, _gizmo_mode, object_matrix, delta_matrix, nullptr);
      break;
    }
    case WMO:
    {
      wmo_instance = boost::get<selected_wmo_type>(selection[0]);
      wmo_instance->recalcExtents();
      object_matrix = wmo_instance->transform_matrix_transposed();
      ImGuizmo::Manipulate(model_view_trs, projection_trs, _gizmo_operation, _gizmo_mode, object_matrix, delta_matrix, nullptr);
      break;
    }
    case MULTISELECTION:
    {
      if (isUsing())
        _last_pivot_scale = ImGuizmo::GetOperationScaleLast();

      ImGuizmo::Manipulate(model_view_trs, projection_trs, _gizmo_operation, _gizmo_mode, pivot_matrix, delta_matrix, nullptr);
      break;
    }
  }

  if (!isUsing())
  {
    return;
  }

  if (gizmo_selection_type == MULTISELECTION)
  {

    for (auto& selected : selection)
    {

      if (selected.which() == eEntry_MapChunk)
        continue;

      if (selected.which() == eEntry_Model)
      {
        model_instance = boost::get<selected_model_type>(selected);
        model_instance->recalcExtents();
        object_matrix = math::matrix_4x4(model_instance->transform_matrix_transposed());
      }
      else if (selected.which() == eEntry_WMO)
      {
        wmo_instance = boost::get<selected_wmo_type>(selected);
        wmo_instance->recalcExtents();
        object_matrix = wmo_instance->transform_matrix_transposed();
      }

      glm::mat4 glm_transform_mat = glm::make_mat4(static_cast<float*>(delta_matrix));

      math::vector_3d& pos = selected.which() == eEntry_Model ? model_instance->pos : wmo_instance->pos;
      math::vector_3d& rotation = selected.which() == eEntry_Model ? model_instance->dir : wmo_instance->dir;
      float wmo_scale = 0.f;
      float& scale = selected.which() == eEntry_Model ? model_instance->scale : wmo_scale;

      glm::vec3 new_scale;
      glm::quat new_orientation;
      glm::vec3 new_translation;
      glm::vec3 new_skew_;
      glm::vec4 new_perspective_;

      glm::decompose(glm_transform_mat,
                     new_scale,
                     new_orientation,
                     new_translation,
                     new_skew_,
                     new_perspective_
      );

      new_orientation = glm::conjugate(new_orientation);

      switch (_gizmo_operation)
      {
        if (_world)
          _world->updateTilesEntry(selection[0], model_update::remove);

        case ImGuizmo::TRANSLATE:
        {
          pos += {new_translation.x, new_translation.y, new_translation.z};
          break;
        }
        case ImGuizmo::ROTATE:
        {
          auto rot_euler = glm::degrees(glm::eulerAngles(new_orientation));
          auto rot_euler_pivot = glm::eulerAngles(new_orientation);

          if (!_use_multiselection_pivot)
          {
            rotation += {rot_euler.x, rot_euler.y, rot_euler.z};
          }
          else
          {
            //LogDebug << rot_euler.x << " " << rot_euler.y << " " << rot_euler.z << std::endl;
            rotation.y += rot_euler.y;

            // building model matrix
            glm::mat4 model_transform = glm::make_mat4(static_cast<float*>(object_matrix));

            // only translation of pivot
            glm::mat4 transformed_pivot = glm::make_mat4(static_cast<float*>(pivot_matrix));

            // model matrix relative to translated pivot
            glm::mat4 model_transform_rel = glm::inverse(transformed_pivot) * model_transform;

            glm::mat4 gizmo_rotation = glm::mat4_cast(new_orientation);

            glm::mat4 _transformed_pivot_rot = transformed_pivot * gizmo_rotation;

            // apply transform to model matrix
            glm::mat4 result_matrix = _transformed_pivot_rot * model_transform_rel;

            glm::vec3 rot_result_scale;
            glm::quat rot_result_orientation;
            glm::vec3 rot_result_translation;
            glm::vec3 rot_result_skew_;
            glm::vec4 rot_result_perspective_;

            glm::decompose(result_matrix,
                           rot_result_scale,
                           rot_result_orientation,
                           rot_result_translation,
                           rot_result_skew_,
                           rot_result_perspective_
            );

            rot_result_orientation = glm::conjugate(rot_result_orientation);

            auto rot_result_orientation_euler = glm::degrees(glm::eulerAngles(rot_result_orientation));

            pos = {rot_result_translation.x, rot_result_translation.y, rot_result_translation.z};
            //rotation = {rot_result_orientation_euler.x, rot_result_orientation_euler.y, rot_result_orientation_euler.z};

          }

          break;
        }
        case ImGuizmo::SCALE:
        {
          scale = std::max(0.001f, scale * (new_scale.x / _last_pivot_scale));
          break;
        }
        case ImGuizmo::BOUNDS:
        {
          throw std::logic_error("Bounds are not supported by this gizmo.");
          break;
        }

        if (gizmo_selection_type == GizmoInternalMode::MODEL)
        {
          model_instance->recalcExtents();
        }
        else
        {
          wmo_instance->recalcExtents();
        }

        if (_world)
         _world->updateTilesEntry(selection[0], model_update::add);
      }
    }
  }
  else
  {
    for (auto& selected : selection)
    {
      if (selected.which() == eEntry_MapChunk)
        continue;

      if (selected.which() == eEntry_Model)
      {
        model_instance = boost::get<selected_model_type>(selected);
        model_instance->recalcExtents();
        object_matrix = math::matrix_4x4(model_instance->transform_matrix_transposed());
      }
      else if (selected.which() == eEntry_WMO)
      {
        wmo_instance = boost::get<selected_wmo_type>(selected);
        wmo_instance->recalcExtents();
        object_matrix = wmo_instance->transform_matrix_transposed();
      }

      glm::mat4 glm_transform_mat = glm::make_mat4(static_cast<float*>(delta_matrix));

      math::vector_3d& pos = gizmo_selection_type == GizmoInternalMode::MODEL ? model_instance->pos : wmo_instance->pos;
      math::vector_3d& rotation = gizmo_selection_type == GizmoInternalMode::MODEL ? model_instance->dir : wmo_instance->dir;
      float wmo_scale = 0.f;
      float& scale = gizmo_selection_type == GizmoInternalMode::MODEL ? model_instance->scale : wmo_scale;

      glm::vec3 new_scale;
      glm::quat new_orientation;
      glm::vec3 new_translation;
      glm::vec3 new_skew_;
      glm::vec4 new_perspective_;

      glm::decompose(glm_transform_mat,
                     new_scale,
                     new_orientation,
                     new_translation,
                     new_skew_,
                     new_perspective_
      );

      new_orientation = glm::conjugate(new_orientation);

      switch (_gizmo_operation)
      {
        if (_world)
          _world->updateTilesEntry(selection[0], model_update::remove);

        case ImGuizmo::TRANSLATE:
        {
          pos += {new_translation.x, new_translation.y, new_translation.z};
          break;
        }
        case ImGuizmo::ROTATE:
        {
          auto rot_euler = glm::eulerAngles(new_orientation) * 57.2957795f;
          rotation += {rot_euler.x, rot_euler.y, rot_euler.z};
          break;
        }
        case ImGuizmo::SCALE:
        {
          scale = std::max(0.001f, new_scale.x);
          break;
        }
        case ImGuizmo::BOUNDS:
        {
          throw std::logic_error("Bounds are not supported by this gizmo.");
          break;
        }

        if (gizmo_selection_type == GizmoInternalMode::MODEL)
        {
          model_instance->recalcExtents();
        }
        else
        {
          wmo_instance->recalcExtents();
        }

        if (_world)
          _world->updateTilesEntry(selection[0], model_update::add);
      }
    }
  }
}
