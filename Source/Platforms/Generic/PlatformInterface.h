#pragma once

#include <string>
#include <Core/Types.h>

#include "Input/InputHandler.h"

/** @brief Base class of all OS Platforms (Win32, Mac, Linux).
 *  This abstracts the implementation of each platform based on its
 *  specific implementation.
 *  This class maintains the lifetime of the engine itself and any input from the user.
 */
class IPlatform
{
public:
	virtual ~IPlatform() = default;

	/**
	 * @brief Creates the main platform-specific window and its menu bar.
	 * @return The resulting status code.
	 */
	virtual int32 create() = 0;

	/**
	 * @brief Displays the main platform-specific window.
	 * @return The resulting status code.
	 */
	virtual int32 show() = 0;

	/**
	 * @brief Constructs a new Engine and the necessary platform-specific components. Initiates the main IPlatform::loop function.
	 * @return The resulting status code.
	 */
	virtual int32 start() = 0;

	/**
	 * @brief Ticks the engine forward and renders the current frame. Once rendered, swaps the front buffer for the current back buffer.
	 * @return The result code of the loop execution.
	 */
	virtual int32 loop() = 0;

	/** @brief TODO: Consider implementation requirements.
	 * @return The resulting status code.
	 */
	virtual int32 paint() = 0;

	/**
	 * @brief Tears down any necessary components created during construction.
	 * @return The resulting status code.
	 */
	virtual int32 end() = 0;

	/**
	 * @brief Swaps the current color buffer into the display buffer.
	 * @return The resulting status code.
	 */
	virtual int32 swapBuffers() = 0;

	/**
	 * @brief Returns if this platform is fully initialized or not.
	 * @return True if the platform is initialized, false otherwise.
	 */
	[[nodiscard]] virtual bool isInitialized() const = 0;

	// Properties
	/**
	 * @brief Returns the size of the main platform window.
	 * @return The size of the main platform window.
	 */
	virtual rectf getSize() = 0;

	/**
	 * @brief Opens a file dialog to select a file to open.
	 * @param outFileName The file name of the file to open.
	 * @return True if the file name can be retrieved, false otherwise.
	 */
	virtual bool getFileDialog(std::string& outFileName, const std::string& filter) = 0;

	/**
	 * @brief Constructs a platform-specific menu bar at the top of the dialog.
	 */
	virtual void constructMenuBar() = 0;

	/**
	 * @brief Sets the specified menu item (defined by EMenuAction) to the specified state.
	 * This is called from Engine::onMenuActionPressed.
	 * @param actionId The menu item ID to set the state of.
	 * @param checkState The new state of the menu item.
	 */
	virtual void setMenuItemChecked(EMenuAction actionId, bool checkState) = 0;

	/**
	 * @brief Displays a modal message box using the platform-specific method of doing so.
	 * @param title The title of the message box.
	 * @param message The text to display within the message box.
	 */
	virtual void messageBox(const std::string& title, const std::string& message) = 0;
};
