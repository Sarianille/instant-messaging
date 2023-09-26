#include "errors.hpp"
#include "imgui.h"

std::optional<const char*> errors::get_error_message(const boost::system::error_code& ec) {
	switch (ec.value()) {
	case connection_aborted:
		return connection_aborted_error_message;
	case server_disconnected:
		return server_disconnected_error_message;
	case connection_timed_out:
		return connection_timed_out_error_message;
	case connection_refused:
		return connection_refused_error_message;
	default:
		return std::nullopt;
	}
}

void errors::error_handler::set_error_message(std::string&& error_message) {
	std::lock_guard<std::mutex> lock(error_message_mutex);
	error_message_ = std::move(error_message);
}

void errors::error_handler::potentially_display_error_message(const char* error_title, const std::function<void()>& on_close) {
	std::lock_guard<std::mutex> lock(error_message_mutex);
	if (error_message_.has_value()) {
		ImGui::OpenPopup(error_title);
		ImGui::BeginPopupModal(error_title, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("%s", error_message_.value().c_str());

		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
			clear_error_message();
			on_close();
		}

		ImGui::EndPopup();
	}
}

void errors::error_handler::clear_error_message() {
	error_message_ = std::nullopt;
}