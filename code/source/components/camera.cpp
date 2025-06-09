#include <camera.hpp>


// #### CAMERA COMPONENT ####

CameraComponent::CameraComponent() {

    is_active_ = true;

    //Default rotation
    pitch_ = 0.0f;
    yaw_ = 0.0f;
    roll_ = 0.0f;

    position_ = glm::vec3(0.0f, -10.0f, 0.0f);

    //forward_dir_ = glm::vec3(0.0f, 0.0f, 1.0f);
    //right_dir_ = glm::vec3(1.0f, 0.0f, 0.0f);
    //up_dir_ = glm::vec3(0.0f, 1.0f, 0.0f);

    view_ = glm::mat4(1.0f);
    //rotation_ = glm::mat4(1.0f);

    fov_ = 90.0f;
    aspect_ratio_ = 1.0f;
    znear_ = 0.1f;
    zfar_ = 100.0f;
    projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
    mode_ = CameraMode::kPerspective;

    //Default init
    left_ = 0.0f;
    right_ = 0.0f;
    bottom_ = 0.0f;
    top_ = 0.0f;

    speed_ = 0.25f;

    //Force first update
    UpdateCamera({}, 0.0f);

}

CameraComponent::CameraComponent(CameraMode mode) {

    is_active_ = true;

    //Default rotations
    pitch_ = 0.0f;
    yaw_ = 0.0f;
    roll_ = 0.0f;

    position_ = glm::vec3(0.0f, -10.0f, 0.0f);

    //forward_dir_ = glm::vec3(0.0f, 0.0f, 1.0f);
    //right_dir_ = glm::vec3(1.0f, 0.0f, 0.0f);
    //up_dir_ = glm::vec3(0.0f, 1.0f, 0.0f);
    view_ = glm::mat4(1.0f);
    //rotation_ = glm::mat4(1.0f);

    //Shared
    znear_ = 0.1f;
    zfar_ = 100.0f;

    //Default init
    left_ = 0.0f;
    right_ = 0.0f;
    bottom_ = 0.0f;
    top_ = 0.0f;
    fov_ = 90.0f;
    aspect_ratio_ = 1.0f;

    speed_ = 0.25f;

    if (mode == CameraMode::kPerspective) {
        fov_ = 90.0f;
        aspect_ratio_ = 1.0f;
        projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
        mode_ = CameraMode::kPerspective;
    }
    else if (mode == CameraMode::kOrthographic) {
        left_ = 0.0f;
        right_ = 1080.0f;
        bottom_ = 720.0f;
        top_ = 0.0f;
        projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
        mode_ = CameraMode::kOrthographic;
    }

    UpdateCamera({}, 0.0f);

}

CameraComponent* CameraComponent::SetOrthographic(float l, float r, float b, float t, float zn, float zf) {
    if (this == nullptr) { return nullptr; }
    left_ = l;
    right_ = r;
    bottom_ = b;
    top_ = t;
    projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
    mode_ = CameraMode::kOrthographic;

    return this;
}

CameraComponent* CameraComponent::SetPerspective(float f, float ar, float zn, float zf) {
    if (this == nullptr) { return nullptr; }
    fov_ = f;
    aspect_ratio_ = ar;
    znear_ = zn;
    zfar_ = zf;
    projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
    mode_ = CameraMode::kPerspective;

    return this;
}

CameraComponent* CameraComponent::UpdateCamera(Input i, float dt) {
    //Stablish the reciproc smoothnes of the joysticks
    static float joystick_smooth = (1.0f / (float)INPUT_DEADZONE);

    if (this == nullptr) { return nullptr; }
    float temp = speed_ * dt;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction.y = sin(glm::radians(pitch_));
    direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    direction = glm::normalize(direction);

    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));

    //Forward or backwards
    if (i.up || i.w || i.dpad_up || i.l_stick_y >= INPUT_DEADZONE) {

        //Prioritize joystick input
        if (i.l_stick_norm_y >= INPUT_DEADZONE) {
            position_ += (direction * temp) * (joystick_smooth * i.l_stick_y);
        }
        else {
            position_ += (direction * temp);
        }
    }
    else if (i.down || i.s || i.dpad_down || i.l_stick_y <= -INPUT_DEADZONE) {

        if (i.l_stick_norm_y <= -INPUT_DEADZONE) {
            position_ -= (direction * temp) * (joystick_smooth * i.l_stick_y);
        }
        else {
            position_ -= (direction * temp);
        }

    }

    //Left or right
    if (i.left || i.a || i.dpad_left || i.l_stick_x <= -INPUT_DEADZONE) {

        if (i.l_stick_norm_x <= -INPUT_DEADZONE) {
            position_ += (right * temp) * (joystick_smooth * i.l_stick_x);
        }
        else {
            position_ += (right * temp);
        }
    }
    else if (i.right || i.d || i.dpad_right || i.l_stick_x >= INPUT_DEADZONE) {
        if (i.l_stick_norm_x >= INPUT_DEADZONE) {
            position_ -= (right * temp) * (joystick_smooth * i.l_stick_x);
        }
        else {
            position_ -= (right * temp);
        }
    }

    //Up or down
    if (i.shift_left || i.button_lb) { position_.y -= (temp); }
    else if (i.space || i.button_rb) { position_.y += (temp); }

    //Jostick right for camera or mouse
    if (i.r_stick_x < -INPUT_DEADZONE || i.r_stick_x > INPUT_DEADZONE ||
        i.r_stick_y < -INPUT_DEADZONE || i.r_stick_y > INPUT_DEADZONE) {
        if (i.r_stick_x < -INPUT_DEADZONE || i.r_stick_x > INPUT_DEADZONE) {
            yaw_ += i.r_stick_norm_x;
        }
        if (i.r_stick_y < -INPUT_DEADZONE || i.r_stick_y > INPUT_DEADZONE) {
            pitch_ += i.r_stick_norm_y;
        }
    }
    else if (i.mouse_right_clicked_) {
        //Yaw
        yaw_ += (float)(i.mouse_x - i.previous_mouse_x);
        //Pitch
        pitch_ -= (float)(i.mouse_y - i.previous_mouse_y);
    }

    //Limit rotations
    if (pitch_ > 89.0f) { pitch_ = 89.0f; }
    else if (pitch_ < -89.0f) { pitch_ = -89.0f; }

    if (yaw_ < -180.0f) { yaw_ += 360.0f; }
    else if (yaw_ > 180.0f) { yaw_ -= 360.0f; }


    view_ = glm::lookAt(position_,
        position_ + direction,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    return this;
}


void CameraComponent::set_mode(CameraMode m) {
    mode_ = m;
}

void CameraComponent::set_pitch(float pitch) {
    pitch_ = pitch;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_yaw(float yaw) {
    yaw_ = yaw;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_roll(float roll) {
    roll_ = roll;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_position(float x, float y, float z) {
    position_ = glm::vec3(x, y, z);
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_near(float n) {
    znear_ = n;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_far(float f) {
    zfar_ = f;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_orthographic_left(float l) {
    left_ = l;
    projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
    UpdateCamera({}, 0.0f);
}
void CameraComponent::set_orthographic_right(float r) {
    right_ = r;
    projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
    UpdateCamera({}, 0.0f);
}
void CameraComponent::set_orthographic_bottom(float b) {
    bottom_ = b;
    projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
    UpdateCamera({}, 0.0f);
}
void CameraComponent::set_orthographic_top(float t) {
    top_ = t;
    projection_ = glm::ortho(left_, right_, bottom_, top_, znear_, zfar_);
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_orthographic_limits(float l, float r, float b, float t) {
    left_ = l;
    right_ = r;
    bottom_ = b;
    top_ = t;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_fov(float fov) {
    fov_ = fov;
    UpdateCamera({}, 0.0f);
    projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, znear_, zfar_);
}

void CameraComponent::set_aspect_ratio(float ar) {
    aspect_ratio_ = ar;
    UpdateCamera({}, 0.0f);
}

void CameraComponent::set_speed(float s) {
    speed_ = s;
    UpdateCamera({}, 0.0f);
}

///////////////////////////////////

CameraMode CameraComponent::get_mode() {
    return mode_;
}

float CameraComponent::get_pitch() {
    return pitch_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_yaw() {
    return yaw_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_roll() {
    return roll_;
    UpdateCamera({}, 0.0f);
}

glm::vec3 CameraComponent::get_position() {
    return position_;
}

glm::mat4 CameraComponent::get_view(){
    return view_;
}

glm::mat4 CameraComponent::get_projection(){
    return projection_;
}

float CameraComponent::get_near() {
    return znear_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_far() {
    return zfar_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_orthographic_left() {
    return left_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_orthographic_right() {
    return right_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_orthographic_bottom() {
    return bottom_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_orthographic_top() {
    return top_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_fov() {
    return fov_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_aspect_ratio() {
    return aspect_ratio_;
    UpdateCamera({}, 0.0f);
}

float CameraComponent::get_speed() {
    return speed_;
    UpdateCamera({}, 0.0f);
}


