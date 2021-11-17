/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#pragma once
#include <dpp/export.h>
#include <dpp/discord.h>
#include <dpp/message.h>
#include <dpp/slashcommand.h>
#include <functional>
#include <variant>
#include <exception>
#include <algorithm>

namespace dpp {

/* Forward declaration */
struct confirmation_callback_t;

/**
 * @brief A function used as a callback for any REST based command
 */
typedef std::function<void(const confirmation_callback_t&)> command_completion_event_t;

/** @brief Base event parameter struct.
 * Each event you receive from the library will have its parameter derived from this class.
 * The class contains the raw event data, and a pointer to the current shard's dpp::discord_client object.
 * You can also use this object to cancel the current event, meaning that any listeners after yours do
 * not get notified of the current event if you call it.
 */
struct DPP_EXPORT event_dispatch_t {
	/**
	 * @brief Raw event data.
	 * If you are using json on your websocket, this will contain json, and if you are using
	 * ETF as your websocket protocol, it will contain raw ETF data.
	 */
	const std::string raw_event;

	/**
	 * @brief Shard the event came from.
	 * Note that for some events, notably voice events, this may be nullptr.
	 */
	class discord_client* from; 
	
	/**
	 * @brief Construct a new event_dispatch_t object
	 * 
 	 * @param client The shard the event originated on. May be a nullptr, e.g. for voice events
	 * @param raw Raw event data as JSON or ETF
	 */
	event_dispatch_t(class discord_client* client, const std::string& raw);

	/**
	 * @brief Cancels the event in progress. Any other attached lambdas for this event after this one are not called.
	 * Note that event cancellation is a thread local state, and not stored in the object (because object which can
	 * be cancelled is `const` during the event, and cannot itself contain the changeable state).
	 * @return const event_dispatch_t& reference to self for chaining
	 */
	const event_dispatch_t& cancel_event() const;

	/**
	 * @brief Returns true if the event is cancelled.
	 * Note that event cancellation is a thread local state, and not stored in the object (because object which can
	 * be cancelled is `const` during the event, and cannot itself contain the changeable state).
	 * @return true if the event is cancelled
	 */
	bool is_cancelled() const;
};

/**
 * @brief Call all listeners for an event handler.
 * 
 * @tparam V a `std::vector` of `std::function`, where each `std::function` takes a single parameter derived from dpp::event_dispatch_t
 * @tparam E an object derived from dpp::event_dispatch_t to pass as the single parameter to the event listener functions
 * @param vec vector of listeners attached to an event handler
 * @param event event object to pass as a parameter to the event handler
 */
template< class V, class E > void call_event(const V& vec, const E& event) {
	std::for_each(vec.begin(), vec.end(), [&](auto &ev) {
		if (!event.is_cancelled()) {
			ev(event);
		}
	});
};


/** @brief Log messages */
struct DPP_EXPORT log_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	log_t(class discord_client* client, const std::string& raw);
	/** Severity */
	loglevel severity;
	/** Log Message */
	std::string message;
};

/** @brief Add user to scheduled event */
struct DPP_EXPORT guild_scheduled_event_user_add_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	guild_scheduled_event_user_add_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief event user added to
	 */
	snowflake event_id;

	/**
	 * @brief User being added
	 * 
	 */
	snowflake user_id;

	/**
	 * @brief Guild being added to
	 * 
	 */
	snowflake guild_id;
};

/** @brief Delete user from scheduled event */
struct DPP_EXPORT guild_scheduled_event_user_remove_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	guild_scheduled_event_user_remove_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief event user removed from
	 */
	snowflake event_id;

	/**
	 * @brief User being removed
	 * 
	 */
	snowflake user_id;

	/**
	 * @brief Guild being removed from
	 * 
	 */
	snowflake guild_id;
};

/** @brief Create scheduled event */
struct DPP_EXPORT guild_scheduled_event_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	guild_scheduled_event_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief created event
	 */
	scheduled_event created;
};

/** @brief Create scheduled event */
struct DPP_EXPORT guild_scheduled_event_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	guild_scheduled_event_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief updated event
	 */
	scheduled_event updated;
};

/** @brief Delete scheduled event */
struct DPP_EXPORT guild_scheduled_event_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	guild_scheduled_event_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief deleted event
	 */
	scheduled_event deleted;
};



/** @brief Create stage instance */
struct DPP_EXPORT stage_instance_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	stage_instance_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief created stage instance
	 */
	stage_instance created;
};

/** @brief Update stage instance */
struct DPP_EXPORT stage_instance_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	stage_instance_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief updated stage instance
	 */
	stage_instance updated;
};

/** @brief Delete stage instance */
struct DPP_EXPORT stage_instance_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on. CAN BE NULL
	 * for log events originating from the cluster object
	 * @param raw Raw event text as JSON
	 */
	stage_instance_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief deleted stage instance
	 */
	stage_instance deleted;
};

/** @brief Voice state update */
struct DPP_EXPORT voice_state_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	voice_state_update_t(class discord_client* client, const std::string& raw);
	/** Voice state */
	voicestate state;
};

/**
 * @brief Create interaction
 */
struct DPP_EXPORT interaction_create_t : public event_dispatch_t {

	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	interaction_create_t(class discord_client* client, const std::string& raw);

	/**
	 * @brief Send a reply for this interaction
	 * 
	 * @param t Type of reply to send
	 * @param m Message object to send. Not all fields are supported by Discord.
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void reply(interaction_response_type t, const message & m, command_completion_event_t callback = {}) const;

	/**
	 * @brief Send a reply for this interaction
	 * 
	 * @param t Type of reply to send
	 * @param mt The string value to send, for simple text only messages
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void reply(interaction_response_type t, const std::string & mt, command_completion_event_t callback = {}) const;

	/**
	 * @brief Get original response message for this interaction
	 *
	 * @param callback Function to call when the API call completes.
	 * On success the callback will contain a dpp::message object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void get_original_response(command_completion_event_t callback) const;

	/**
	 * @brief Edit the response for this interaction
	 *
	 * @param m Message object to send. Not all fields are supported by Discord.
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void edit_response(const message & m, command_completion_event_t callback = {}) const;

	/**
	 * @brief Edit the response for this interaction
	 *
	 * @param mt The string value to send, for simple text only messages
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void edit_response(const std::string & mt, command_completion_event_t callback = {}) const;

	/**
	 * @brief Delete the original response for this interaction
	 *
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void delete_original_response(command_completion_event_t callback = {});

	/**
	 * @brief Set the bot to 'thinking' state
	 *
	 * @param callback User function to execute when the api call completes.
	 * On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true. You can obtain full error details with confirmation_callback_t::get_error().
	 */
	void thinking(command_completion_event_t callback = {}) const;

	/**
	 * @brief Get a command line parameter
	 * 
	 * @param name The command line parameter to retrieve
	 * @return const command_value& If the command line parameter does not 
	 * exist, an empty variant is returned.
	 */
	const virtual command_value& get_parameter(const std::string& name) const;

	/**
	 * @brief command interaction
	 */
	interaction command;
};

/**
 * @brief Click on button
 */
struct DPP_EXPORT button_click_t : public interaction_create_t {

	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	button_click_t(class discord_client* client, const std::string& raw);

	/**
	 * @brief Get a command line parameter
	 * 
	 * @param name The command line parameter to retrieve
	 * @return Always returns an empty parameter as buttons dont have parameters!
	 */
	const virtual command_value& get_parameter(const std::string& name) const;
	/**
	 * @brief button custom id
	 */
	std::string custom_id;
	/**
	 * @brief component type
	 */
	uint8_t component_type;
};

/**
 * @brief Discord requests that we fill a list of auto completion choices for a command option
 */
struct DPP_EXPORT autocomplete_t : public interaction_create_t {

	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	autocomplete_t(class discord_client* client, const std::string& raw);

	/**
	 * @brief Get a command line parameter
	 * 
	 * @param name The command line parameter to retrieve
	 * @return Always returns an empty parameter as auto complete requests dont have parameters!
	 */
	const virtual command_value& get_parameter(const std::string& name) const;

	/**
	 * @brief Command ID
	 */
	dpp::snowflake id;

	/**
	 * @brief Command name
	 */
	std::string name;

	/**
	 * @brief auto completion options
	 */
	std::vector<dpp::command_option> options;
};


/**
 * @brief Click on select
 */
struct DPP_EXPORT select_click_t : public interaction_create_t {

	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	select_click_t(class discord_client* client, const std::string& raw);

	/**
	 * @brief Get a command line parameter
	 * 
	 * @param name The command line parameter to retrieve
	 * @return Always returns an empty parameter as buttons dont have parameters!
	 */
	const virtual command_value& get_parameter(const std::string& name) const;

	/**
	 * @brief select menu custom id
	 */
	std::string custom_id;
	/**
	 * @brief select menu values
	 */
	std::vector<std::string> values;
	/**
	 * @brief select menu component type
	 */
	uint8_t component_type;
};


/** @brief Delete guild */
struct DPP_EXPORT guild_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_delete_t(class discord_client* client, const std::string& raw);
	/** Deleted guild */
	guild* deleted;
};

/** @brief Update guild stickers */
struct DPP_EXPORT guild_stickers_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_stickers_update_t(class discord_client* client, const std::string& raw);
	/** Updating guild */
	guild* updating_guild;
	/**
	 * @brief stickers being updated
	 */
	std::vector<sticker> stickers;
};

/** @brief Guild join request delete (user declined membership screening) */
struct DPP_EXPORT guild_join_request_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_join_request_delete_t(class discord_client* client, const std::string& raw);
	/** Deleted guild */
	snowflake guild_id;
	/**
	 * @brief user id
	 */
	snowflake user_id;
};

/** @brief Delete channel */
struct DPP_EXPORT channel_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	channel_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild channel is being deleted from
	 */
	guild* deleting_guild;
	/**
	 * @brief channel being deleted
	 */
	channel* deleted;
};

/** @brief Update channel */
struct DPP_EXPORT channel_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	channel_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild channel is being updated on
	 */
	guild* updating_guild;
	/**
	 * @brief channel being updated
	 */
	channel* updated;
};

/** @brief Session ready */
struct DPP_EXPORT ready_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	ready_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief websocket session id
	 */
	std::string session_id;
	/**
	 * @brief shard id
	 */
	uint32_t shard_id;
};

/** @brief Message Deleted */
struct DPP_EXPORT message_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief message being deleted
	 */
	message* deleted;
};

/** @brief Application slash command deleted */
struct DPP_EXPORT application_command_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	application_command_delete_t(class discord_client* client, const std::string& raw);
};

/** @brief Guild member remove */
struct DPP_EXPORT guild_member_remove_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_member_remove_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild user is being removed from
	 */
	guild* removing_guild;
	/**
	 * @brief user being removed
	 */
	user* removed;
};

/**
 * @brief Create application slash command
 * 
 */
struct DPP_EXPORT application_command_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	application_command_create_t(class discord_client* client, const std::string& raw);
};

/** @brief Session resumed */
struct DPP_EXPORT resumed_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	resumed_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief websocket session id
	 */
	std::string session_id;
	/**
	 * @brief shard id
	 */
	uint32_t shard_id;
};

/** @brief Guild role create */
struct DPP_EXPORT guild_role_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_role_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild role is being created on
	 */
	guild* creating_guild;
	/**
	 * @brief role being created
	 */
	role* created;
};

/** @brief Typing start */
struct DPP_EXPORT typing_start_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	typing_start_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild user is typing on
	 */
	guild* typing_guild;
	/**
	 * @brief channel user is typing on
	 */
	channel* typing_channel;
	/**
	 * @brief user who is typing.
	 * Can be nullptr if user is not cached
	 */
	user* typing_user;
	/**
	 * @brief User id of user typing.
	 * Always set regardless of caching
	 */
	snowflake user_id;
	/**
	 * @brief Time of typing event
	 */
	time_t timestamp;
};

/** @brief Voice state update */
struct DPP_EXPORT voice_track_marker_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on.
	 * Will always be null.
	 * @param raw Raw event text as JSON.
	 * Will always be empty.
	 */
	voice_track_marker_t(class discord_client* client, const std::string& raw);
	/** Voice client */
	class discord_voice_client* voice_client;
	/** Track metadata */
	std::string track_meta;
};


/** @brief Message reaction add */
struct DPP_EXPORT message_reaction_add_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_reaction_add_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief Guild reaction occurred on
	 */
	guild* reacting_guild;
	/**
	 * @brief User who reacted
	 */
	user reacting_user;
	/**
	 * @brief member data of user who reacted
	 */
	guild_member reacting_member;
	/**
	 * @brief channel the reaction happened on
	 */
	channel* reacting_channel;
	/**
	 * @brief emoji of reaction
	 */
	emoji reacting_emoji;
	/**
	 * @brief message id of the message reacted upon
	 */
	snowflake message_id;
};

/** @brief Guild members chunk */
struct DPP_EXPORT guild_members_chunk_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_members_chunk_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild the members chunk is for
	 */
	guild* adding;
	/**
	 * @brief list of members in the chunk
	 */
	guild_member_map* members;
};

/** @brief Message reaction remove */
struct DPP_EXPORT message_reaction_remove_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_reaction_remove_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief Guild reaction occurred on
	 */
	guild* reacting_guild;
	/**
	 * @brief User who reacted
	 */
	dpp::snowflake reacting_user_id;
	/**
	 * @brief channel the reaction happened on
	 */
	channel* reacting_channel;
	/**
	 * @brief emoji of reaction
	 */
	emoji reacting_emoji;
	/**
	 * @brief message id of the message reacted upon
	 */
	snowflake message_id;	
};

/** @brief Create guild */
struct DPP_EXPORT guild_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild that was created
	 */
	guild* created;
};

/** @brief Create channel */
struct DPP_EXPORT channel_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	channel_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild channel was created on
	 */
	guild* creating_guild;
	/**
	 * @brief channel that was created
	 */
	channel* created;
};

/** @brief Message remove emoji */
struct DPP_EXPORT message_reaction_remove_emoji_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_reaction_remove_emoji_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief Guild reaction occurred on
	 */
	guild* reacting_guild;
	/**
	 * @brief channel the reaction happened on
	 */
	channel* reacting_channel;
	/**
	 * @brief emoji of reaction
	 */
	emoji reacting_emoji;
	/**
	 * @brief message id of the message reacted upon
	 */
	snowflake message_id;	
};

/** @brief Message delete bulk */
struct DPP_EXPORT message_delete_bulk_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_delete_bulk_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild messages are being deleted upon
	 */
	guild* deleting_guild;
	/**
	 * @brief user who is deleting the messages
	 */
	user* deleting_user;
	/**
	 * @brief channel messages are being deleted from
	 */
	channel* deleting_channel;
	/**
	 * @brief list of message ids of deleted messages
	 */
	std::vector<snowflake> deleted;
};

/** @brief Guild role update */
struct DPP_EXPORT guild_role_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_role_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where roles are being updated
	 */
	guild* updating_guild;
	/**
	 * @brief the role being updated
	 */
	role* updated;
};

/** @brief Guild role delete */
struct DPP_EXPORT guild_role_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_role_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where role is being deleted
	 */
	guild* deleting_guild;
	/**
	 * @brief role being deleted
	 */
	role* deleted;
};

/** @brief Channel pins update */
struct DPP_EXPORT channel_pins_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	channel_pins_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where message is bieng pinned
	 */
	guild* pin_guild;
	/**
	 * @brief channel where message is being pinned
	 */
	channel* pin_channel;
	/**
	 * @brief timestamp of pin
	 */
	time_t timestamp;
};

/** @brief Message remove all reactions */
struct DPP_EXPORT message_reaction_remove_all_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_reaction_remove_all_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief Guild reaction occurred on
	 */
	guild* reacting_guild;
	/**
	 * @brief channel the reaction happened on
	 */
	channel* reacting_channel;
	/**
	 * @brief message id of the message reacted upon
	 */
	snowflake message_id;	

};

/** @brief Voice server update */
struct DPP_EXPORT voice_server_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	voice_server_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild id where voice server updated
	 */
	snowflake guild_id;
	/**
	 * @brief voice server token, used to connect to vc
	 */
	std::string token;
	/**
	 * @brief voice server endpoint wss:// address
	 * 
	 */
	std::string endpoint;
};

/** @brief Guild emojis update */
struct DPP_EXPORT guild_emojis_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_emojis_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief snowflake ids of list of emojis
	 */
	std::vector<snowflake> emojis;
	/**
	 * @brief guild where emojis are being updated
	 */
	guild* updating_guild;
};

/**
 * @brief Presence update
 * 
 */
struct DPP_EXPORT presence_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	presence_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief rich presence being updated
	 */
	presence rich_presence;
};

/** @brief Webhooks update */
struct DPP_EXPORT webhooks_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	webhooks_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where webhooks are being updated
	 */
	guild* webhook_guild;
	/**
	 * @brief channel where webhooks are being updated
	 */
	channel* webhook_channel;
};

/** @brief Guild member add */
struct DPP_EXPORT guild_member_add_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_member_add_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild which gained new member
	 */
	guild* adding_guild;
	/**
	 * @brief member which was added
	 */
	guild_member added;
};

/** @brief Invite delete */
struct DPP_EXPORT invite_delete_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	invite_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief the deleted invite
	 */
	invite deleted_invite;
};

/** @brief Guild update */
struct DPP_EXPORT guild_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild being updated
	 */
	guild* updated;
};

/** @brief Guild integrations update */
struct DPP_EXPORT guild_integrations_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_integrations_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where integrations are being updated
	 */
	guild* updating_guild;
};

/** @brief Guild member update */
struct DPP_EXPORT guild_member_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_member_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where member is being updated
	 */
	guild* updating_guild;
	/**
	 * @brief member being updated
	 */
	guild_member updated;
};

/**
 * @brief Update application slash command
 * 
 */
struct DPP_EXPORT application_command_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	application_command_update_t(class discord_client* client, const std::string& raw);
};

/** @brief Invite create */
struct DPP_EXPORT invite_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	invite_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief created invite
	 */
	invite created_invite;
};

/** @brief Message update */
struct DPP_EXPORT message_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief message being updated
	 */
	message* updated;
};

/** @brief User update */
struct DPP_EXPORT user_update_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	user_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief user being updated
	 */
	user updated;
};

/** @brief Create message */
struct DPP_EXPORT message_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	message_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief message that was created (sent).
	 * If you use any parts of this in another thread, take a copy! It doesn't stick around!
	 */
	message* msg;
	/**
	 * @brief Send a text to the same channel as the channel_id in recieved event.
	 * @param m Text to send
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void send(const std::string& m, command_completion_event_t callback = {}) const;
	/**
	 * @brief Send a message to the same channel as the channel_id in recieved event.
	 * @param msg Message to send
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void send(message& msg, command_completion_event_t callback = {}) const;
	/**
	 * @brief Send a message to the same channel as the channel_id in recieved event.
	 * @param msg Message to send
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void send(message&& msg, command_completion_event_t callback = {}) const;
	/**
	 * @brief Reply to the message recieved in the event.
	 * @param m Text to send
	 * @param mention_replied_user mentions (pings) the author of message replied to, if true
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void reply(const std::string& m, bool mention_replied_user = false, command_completion_event_t callback = {}) const;
	/**
	 * @brief Reply to the message recieved in the event.
	 * @param msg Message to send as a reply.
	 * @param mention_replied_user mentions (pings) the author of message replied to, if true
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void reply(message& msg, bool mention_replied_user = false, command_completion_event_t callback = {}) const;
	/**
	 * @brief Reply to the message recieved in the event.
	 * @param msg Message to send as a reply.
	 * @param mention_replied_user mentions (pings) the author of message replied to, if true
	 * @param callback User function to execute once the API call completes.
	 * @note confirmation_callback_t::value contains a message object on success. On failure, value is undefined and confirmation_callback_t::is_error() is true.
	 */
	void reply(message&& msg, bool mention_replied_user = false, command_completion_event_t callback = {}) const;
};

/** @brief Guild ban add */
struct DPP_EXPORT guild_ban_add_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_ban_add_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where ban was added
	 */
	guild* banning_guild;
	/**
	 * @brief user being banned
	 */
	user banned;
};

/** @brief Guild ban remove */
struct DPP_EXPORT guild_ban_remove_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	guild_ban_remove_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where ban is being removed
	 */
	guild* unbanning_guild;
	/**
	 * @brief user being unbanned
	 */
	user unbanned;
};

/** @brief Integration create */
struct DPP_EXPORT integration_create_t : public event_dispatch_t {
	/** Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	integration_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief created integration
	 */
	integration created_integration;
};

/** @brief Integration update */
struct DPP_EXPORT integration_update_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	integration_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief updated integration
	 */
	integration updated_integration;
};

/** @brief Integration delete */
struct DPP_EXPORT integration_delete_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	integration_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief deleted integration
	 */
	integration deleted_integration;
};

/** @brief Thread Create*/
struct DPP_EXPORT thread_create_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_create_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where thread was created
	 */
	guild* creating_guild;
	/**
	 * @brief thread created
	 */
	thread created;
};

/** @brief Thread Update
*/
struct DPP_EXPORT thread_update_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where thread was updated
	 */
	guild* updating_guild;
	/**
	 * @brief thread updated
	 */
	thread updated;
};

/** @brief Thread Delete
 */
struct DPP_EXPORT thread_delete_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_delete_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where thread was deleted
	 */
	guild* deleting_guild;
	/**
	 * @brief thread deleted
	 */
	thread deleted;
};

/** @brief Thread List Sync
 */
struct DPP_EXPORT thread_list_sync_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_list_sync_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief guild where thread list was synchronised
	 */
	guild* updating_guild;
	/**
	 * @brief list of threads (channels) synchronised
	 */
	std::vector<thread> threads;
	/**
	 * @brief list of thread members for the channels (threads)
	 */
	std::vector<thread_member> members;
};

/** @brief Thread Member Update
 */
struct DPP_EXPORT thread_member_update_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_member_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief updated thread member
	 */
	thread_member updated;
};

/** @brief Thread Members Update
 */
struct DPP_EXPORT thread_members_update_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * @param raw Raw event text as JSON
	 */
	thread_members_update_t(class discord_client* client, const std::string& raw);
	/**
	 * @brief thread (channel) id
	 */
	snowflake thread_id;
	/**
	 * @brief guild thread members updated on
	 */
	guild* updating_guild;
	/**
	 * @brief new approximate member count
	 */
	uint8_t member_count;
	/**
	 * @brief added members
	 */
	std::vector<thread_member> added;
	/**
	 * @brief ids only of removed members
	 */
	std::vector<snowflake> removed_ids;
};

/** @brief voice buffer send
 */
struct DPP_EXPORT voice_buffer_send_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_buffer_send_t(class discord_client* client, const std::string &raw);
	/**
	 * @brief voice client where buffer was sent
	 */
	class discord_voice_client* voice_client;
	/**
	 * @brief encoded size of sent buffer
	 */
	int buffer_size;
};

/** @brief voice user talking */
struct DPP_EXPORT voice_user_talking_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_user_talking_t(class discord_client* client, const std::string &raw);
	/**
	 * @brief voice client where user is talking
	 */
	class discord_voice_client* voice_client;
	/**
	 * @brief talking user id
	 */
	snowflake user_id;
	/**
	 * @brief flags for talking user
	 */
	uint8_t talking_flags;
};

/** @brief voice user talking */
struct DPP_EXPORT voice_ready_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_ready_t(class discord_client* client, const std::string &raw);
	/**
	 * @brief voice client which is ready
	 */
	class discord_voice_client* voice_client;
	/**
	 * @brief id of voice channel
	 */
	snowflake voice_channel_id;
};

/** @brief voice receive packet */
struct DPP_EXPORT voice_receive_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on.
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_receive_t(class discord_client* client, const std::string &raw);
	class discord_voice_client* voice_client;
	/**
	 * @brief Audio data, encoded as 48kHz stereo PCM or Opus
	 */
	uint8_t* audio;
	/**
	 * @brief Size of audio buffer
	 */
	size_t audio_size;
	/**
	 * @brief User ID of speaker (zero if unknown)
	 */
	snowflake user_id;
};

/** @brief voice client speaking event */
struct DPP_EXPORT voice_client_speaking_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on.
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_client_speaking_t(class discord_client* client, const std::string &raw);
	/**
	 * @brief voice client where user is speaking
	 */
	class discord_voice_client* voice_client;
	/**
	 * @brief speaking user id
	 * 
	 */
	snowflake user_id;
	/**
	 * @brief ssrc value of speaking user
	 */
	uint32_t ssrc;
};

/** @brief voice client disconnect event */
struct DPP_EXPORT voice_client_disconnect_t : public event_dispatch_t {
	/** 
	 * @brief Constructor
	 * @param client The shard the event originated on.
	 * WILL ALWAYS be NULL.
	 * @param raw Raw event text as JSON
	 */
	voice_client_disconnect_t(class discord_client* client, const std::string &raw);
	/**
	 * @brief voice client where user disconnected
	 */
	class discord_voice_client* voice_client;
	/**
	 * @brief user id of user who left vc
	 */
	snowflake user_id;
};

/** @brief The dispatcher class contains a set of std::functions representing hooked events
 * that the user code is interested in. These are modified via the on_eventname style
 * methods in the cluster class.
 */
class DPP_EXPORT dispatcher {
public:
	/** @brief Event handler function pointer for log event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const log_t& event)>> log;
	/** @brief Event handler function pointer for voice state update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_state_update_t& event)>> voice_state_update;
	/** @brief Event handler function pointer for voice client speaking event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_client_speaking_t& event)>> voice_client_speaking;
	/** @brief Event handler function pointer for voice client disconnect event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_client_disconnect_t& event)>> voice_client_disconnect;
	/** @brief Event handler function pointer for interaction create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const interaction_create_t& event)>> interaction_create;
	/** @brief Event handler function pointer for button click event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const button_click_t& event)>> button_click;
	/** @brief Event handler function pointer for autocomplete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const autocomplete_t& event)>> autocomplete;
	/** @brief Event handler function pointer for button click event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const select_click_t& event)>> select_click;
	/** @brief Event handler function pointer for guild delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_delete_t& event)>> guild_delete;
	/** @brief Event handler function pointer for channel delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const channel_delete_t& event)>> channel_delete;
	/** @brief Event handler function pointer for channel update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const channel_update_t& event)>> channel_update;
	/** @brief Event handler function pointer for ready event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const ready_t& event)>> ready;
	/** @brief Event handler function pointer for message delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_delete_t& event)>> message_delete;
	/** @brief Event handler function pointer for application command delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const application_command_delete_t& event)>> application_command_delete;
	/** @brief Event handler function pointer for guild member remove event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_member_remove_t& event)>> guild_member_remove;
	/** @brief Event handler function pointer for guild member remove event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const application_command_create_t& event)>> application_command_create;
	/** @brief Event handler function pointer for resumed event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const resumed_t& event)>> resumed;
	/** @brief Event handler function pointer for guild role create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_role_create_t& event)>> guild_role_create;
	/** @brief Event handler function pointer for typing start event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const typing_start_t& event)>> typing_start;
	/** @brief Event handler function pointer for message reaction add event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_reaction_add_t& event)>> message_reaction_add;
	/** @brief Event handler function pointer for guild members chunk event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_members_chunk_t& event)>> guild_members_chunk;
	/** @brief Event handler function pointer for message reaction remove event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_reaction_remove_t& event)>> message_reaction_remove;
	/** @brief Event handler function pointer for guild create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_create_t& event)>> guild_create;
	/** @brief Event handler function pointer for guild channel create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const channel_create_t& event)>> channel_create;
	/** @brief Event handler function pointer for message reaction remove emoji event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_reaction_remove_emoji_t& event)>> message_reaction_remove_emoji;
	/** @brief Event handler function pointer for message delete bulk event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_delete_bulk_t& event)>> message_delete_bulk;
	/** @brief Event handler function pointer for guild role update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_role_update_t& event)>> guild_role_update;
	/** @brief Event handler function pointer for guild role delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_role_delete_t& event)>> guild_role_delete;
	/** @brief Event handler function pointer for channel pins update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const channel_pins_update_t& event)>> channel_pins_update;
	/** @brief Event handler function pointer for message reaction remove all event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_reaction_remove_all_t& event)>> message_reaction_remove_all;
	/** @brief Event handler function pointer for voice server update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_server_update_t& event)>> voice_server_update;
	/** @brief Event handler function pointer for guild emojis update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_emojis_update_t& event)>> guild_emojis_update;
	/** @brief Event handler function pointer for presence update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const presence_update_t& event)>> presence_update;
	/** @brief Event handler function pointer for webhooks update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const webhooks_update_t& event)>> webhooks_update;
	/** @brief Event handler function pointer for guild member add event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_member_add_t& event)>> guild_member_add;
	/** @brief Event handler function pointer for invite delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const invite_delete_t& event)>> invite_delete;
	/** @brief Event handler function pointer for guild update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_update_t& event)>> guild_update;
	/** @brief Event handler function pointer for guild integrations update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_integrations_update_t& event)>> guild_integrations_update;
	/** @brief Event handler function pointer for guild member update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_member_update_t& event)>> guild_member_update;
	/** @brief Event handler function pointer for application command update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const application_command_update_t& event)>> application_command_update;
	/** @brief Event handler function pointer for invite create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const invite_create_t& event)>> invite_create;
	/** @brief Event handler function pointer for message update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_update_t& event)>> message_update;
	/** @brief Event handler function pointer for user update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const user_update_t& event)>> user_update;
	/** @brief Event handler function pointer for message create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const message_create_t& event)>> message_create;
	/** @brief Event handler function pointer for guild ban add event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_ban_add_t& event)>> guild_ban_add;
	/** @brief Event handler function pointer for guild ban remove event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_ban_remove_t& event)>> guild_ban_remove;
	/** @brief Event handler function pointer for integration create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const integration_create_t& event)>> integration_create;
	/** @brief Event handler function pointer for integration update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const integration_update_t& event)>> integration_update;
	/** @brief Event handler function pointer for integration delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const integration_delete_t& event)>> integration_delete;	
	/** @brief Event handler function pointer for thread create event 
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_create_t& event)>> thread_create;
	/** @brief Event handler function pointer for thread update event 
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_update_t& event)>> thread_update;
	/** @brief Event handler function pointer for thread delete event 
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_delete_t& event)>> thread_delete;
	/** @brief Event handler function pointer for thread list sync event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_list_sync_t& event)>> thread_list_sync;
	/** @brief Event handler function pointer for thread member update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_member_update_t& event)>> thread_member_update;
	/** @brief Event handler function pointer for thread members update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const thread_members_update_t& event)>> thread_members_update;
	/** @brief Event handler function pointer for voice buffer send event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_buffer_send_t& event)>> voice_buffer_send;
	/** @brief Event handler function pointer for voice user talking event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_user_talking_t& event)>> voice_user_talking;
	/** @brief Event handler function pointer for voice ready event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_ready_t& event)>> voice_ready;
	/** @brief Event handler function pointer for voice receive event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_receive_t& event)>> voice_receive;
	/** @brief Event handler function pointer for voice track marker event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const voice_track_marker_t& event)>> voice_track_marker;
	/** @brief Event handler function pointer for guild join request delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_join_request_delete_t& event)>> guild_join_request_delete;
	/** @brief Event handler function pointer for stage instance create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const stage_instance_create_t& event)>> stage_instance_create;
	/** @brief Event handler function pointer for stage instance update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const stage_instance_update_t& event)>> stage_instance_update;
	/** @brief Event handler function pointer for stage instance delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const stage_instance_delete_t& event)>> stage_instance_delete;
	/** @brief Event handler function pointer for guild sticker update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_stickers_update_t& event)>> stickers_update;
	/** @brief Event handler function pointer for guild scheduled event create event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_scheduled_event_create_t& event)>> guild_scheduled_event_create;
	/** @brief Event handler function pointer for guild scheduled event update event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_scheduled_event_update_t& event)>> guild_scheduled_event_update;
	/** @brief Event handler function pointer for guild scheduled event delete event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_scheduled_event_delete_t& event)>> guild_scheduled_event_delete;
	/** @brief Event handler function pointer for guild scheduled event user add event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_scheduled_event_user_add_t& event)>> guild_scheduled_event_user_add;
	/** @brief Event handler function pointer for guild scheduled event user remove event
	 * @param event Event parameters
	 */
	std::vector<std::function<void(const guild_scheduled_event_user_remove_t& event)>> guild_scheduled_event_user_remove;
};

};

