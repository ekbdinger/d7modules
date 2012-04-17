
(function ($) {

Drupal.chatroom = Drupal.chatroom || {'initialised' : false, chats: {}};

Drupal.chatroom.initialiseChat = function(chat) {
  if (chat.latestMsgId > 0) {
    var targetOffset = $('#chatroom-board-' + chat.cid + ' div.new-message:last').offset().top;
    var boardOffset = $('#chatroom-board-' + chat.cid).offset().top;
    var scrollAmount = targetOffset - boardOffset;
    $('#chatroom-board-' + chat.cid).animate({scrollTop: '+='+ scrollAmount +'px'}, 500);
    $('#chatroom-board-' + chat.cid + '.new-message').removeClass('new-message');
  }

  $('#edit-chatroom-message-entry-box-' + chat.cid).keyup(function(e) {
    var chat = Drupal.settings.chatroom.chats[this.id.replace(/^edit-chatroom-message-entry-box-/, '')];
    var messageText = $('#edit-chatroom-message-entry-box-' + chat.cid).val().replace(/^\s+|\s+$/g, '');
    var anonNameText = '';
    if (messageText && e.keyCode == 13 && !e.shiftKey && !e.ctrlKey) {
      Drupal.chatroom.postMessage(messageText, anonNameText, chat);
      $('#edit-chatroom-message-entry-box-' + chat.cid).val('').focus();
    }
    else {
      return true;
    }
  });

  $('#edit-chatroom-message-entry-submit-' + chat.cid).click(function (e) {
    var chat = Drupal.settings.chatroom.chats[this.id.replace(/^edit-chatroom-message-entry-submit-/, '')];
    e.preventDefault();
    e.stopPropagation();
    var messageText = $('#edit-chatroom-message-entry-box-' + chat.cid).val().replace(/^\s+|\s+$/g, '');
    var anonNameText = '';
    if (messageText) {
      Drupal.chatroom.postMessage(messageText, anonNameText, chat);
      $('#edit-chatroom-message-entry-box-' + chat.cid).val('').focus();
    }
  });
}

/**
 * We depend on the Nodejs module successfully create a socket for us.
 */
Drupal.Nodejs.connectionSetupHandlers.chatroom = {
  connect: function () {
    for (var cid in Drupal.settings.chatroom.chats) {
      Drupal.chatroom.initialiseChat(Drupal.settings.chatroom.chats[cid]);
    }
    Drupal.chatroom.initialised = true;
  }
};

Drupal.chatroom.postMessage = function(message, anonName, chat) {
  $.ajax({
    type: 'POST',
    url: Drupal.settings.chatroom.postMessagePath + '/' + chat.cid,
    dataType: 'json',
    success: function () {},
    data: {
      message: message,
      anonName: anonName,
      formToken: $('#edit-chatroom-chat-buttons-form-token-' + chat.cid).val(),
      formId: 'chatroom_form_token_' + chat.cid
    }
  });
}

Drupal.chatroom.updateUserList = function(message) {
  if ($('#chatroom-user-' + message.cid + '-' + message.uid).length == 0) {
    $('#chatroom-irc-user-list-' + message.cid).append('<li id="chatroom-user-' + message.cid + '-' + message.uid + '"><a href="/user/' + message.uid + '">' + message.name + '</a></li>');
  }
}

Drupal.chatroom.addMessageToBoard = function(message) {
  $('#chatroom-board-' + message.cid).append(message.msg);
  Drupal.chatroom.scrollToLatestMessage(message.cid);
}

Drupal.chatroom.scrollToLatestMessage = function(cid) {
  var boardOffset = $('#chatroom-board-' + cid).offset().top;
  var targetOffset = $('#chatroom-board-' + cid + ' div.new-message:last').offset().top;
  var scrollAmount = targetOffset - boardOffset;
  $('#chatroom-board-' + cid).animate({scrollTop: '+='+ scrollAmount +'px'}, 500);
  $('#chatroom-board-' + cid + ' .new-message').removeClass('new-message');
}

Drupal.Nodejs.callbacks.chatroomMessageHandler = {
  callback: function (message) {
    Drupal.chatroom.addMessageToBoard(message.data);
  }
};

Drupal.Nodejs.callbacks.chatroomUserOnlineHandler = {
  callback: function (message) {
    Drupal.chatroom.updateUserList(message.data);
  }
};

Drupal.Nodejs.contentChannelNotificationCallbacks.chatroom = {
  callback: function (message) {
    if (message.data.type == 'disconnect') {
      var cid = message.channel.replace(/^chatroom_/, '');
      if ($('#chatroom-user-' + cid + '-' + message.data.uid).length == 1) {
        $('#chatroom-user-' + cid + '-' + message.data.uid).remove();
      }
    }
  }
};

})(jQuery);

// vi:ai:expandtab:sw=2 ts=2

