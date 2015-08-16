// @NOTE: This is a tmp file for experimentation purposes only. These handlers
// should be placed in a more appropriate service before merging this branch.

#ifndef SRC_ZONESERVER_OCCOMMONHANDLERS_H_
#define SRC_ZONESERVER_OCCOMMONHANDLERS_H_


class Message;
class Object;
class ObjectControllerCmdProperties;

/// This command is used to move items about a structure.
/**
 * This command is invoked by the client to gain a temporary burst of speed.
 *
 * The client enters the message in the following format:
 *   /burstRun
 *
 * @param object The object attempting to burst run.
 * @param target No target is set for this command, this value is always a nullptr.
 * @param message The message from the client requesting this command.
 * @param cmd_properties Contextual information for use during processing this command.
 */
bool HandleBurstRun(Object* object, Object* target, Message* message, ObjectControllerCmdProperties* cmdProperties);

#endif  // SRC_ZONESERVER_OCCOMMONHANDLERS_H_
