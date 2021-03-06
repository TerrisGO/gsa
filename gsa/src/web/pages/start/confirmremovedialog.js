/* Greenbone Security Assistant
 *
 * Authors:
 * Björn Ricks <bjoern.ricks@greenbone.net>
 *
 * Copyright:
 * Copyright (C) 2018 Greenbone Networks GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */
import React from 'react';

import glamorous from 'glamorous';

import _ from 'gmp/locale';

import PropTypes from 'web/utils/proptypes';

import Dialog from 'web/components/dialog/dialog';
import DialogContent from 'web/components/dialog/content';
import DialogTitle from 'web/components/dialog/title';
import Button from 'web/components/dialog/button';

import Divider from 'web/components/layout/divider';

const StyledDivider = glamorous(Divider)({
  borderWidth: '1px 0 0 0',
  borderStyle: 'solid',
  borderColor: '#ddd',
  marginTop: '15px',
  padding: '10px 20px 10px 15px',
});

const Content = glamorous.div({
  padding: '5px',
});

const ConfirmRemoveDialog = ({
  dashboardTitle,
  dashboardId,
  onConfirm,
  onDeny,
}) => (
  <Dialog
    width="450px"
    minHeight={100}
    minWidth={200}
    onClose={onDeny}
  >
    {({
      close,
      moveProps,
      heightProps,
    }) => (
      <DialogContent>
        <DialogTitle
          title={_('Remove Dashboard {{name}}', {name: dashboardTitle})}
          onCloseClick={onDeny}
          {...moveProps}
        />
        <Content>
          {_('Do you really want to remove the Dashboard {{name}} and its ' +
             'configuration?', {name: dashboardTitle})}
        </Content>
        <StyledDivider
          align={['end', 'center']}
          shrink="0"
          grow
        >
          <Button
            title={_('Remove')}
            onClick={() => onConfirm(dashboardId)}
          >
            {_('Remove')}
          </Button>
          <Button
            title={_('Abort')}
            onClick={onDeny}
          >
            {_('Abort')}
          </Button>
        </StyledDivider>
      </DialogContent>
    )}
  </Dialog>
);

ConfirmRemoveDialog.propTypes = {
  dashboardId: PropTypes.string.isRequired,
  dashboardTitle: PropTypes.string.isRequired,
  onConfirm: PropTypes.func.isRequired,
  onDeny: PropTypes.func.isRequired,
};

export default ConfirmRemoveDialog;

// vim: set ts=2 sw=2 tw=80:
