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
import {is_function} from 'gmp/utils/identity';

import Filter from 'gmp/models/filter';

import {
  types,
  createEntitiesActions,
  createLoadEntities,
  createEntityActions,
  createLoadEntity,
} from '../actions';

describe('entities actions tests', () => {

  describe('createEntitiesActions tests', () => {

    test('should create action creators for loading', () => {
      const actions = createEntitiesActions('foo');

      expect(actions.request).toBeDefined();
      expect(is_function(actions.request)).toBe(true);
      expect(actions.success).toBeDefined();
      expect(is_function(actions.success)).toBe(true);
      expect(actions.error).toBeDefined();
      expect(is_function(actions.error)).toBe(true);
    });

    test('should create a load request action', () => {
      const actions = createEntitiesActions('foo');
      const action = actions.request();

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_REQUEST,
        entityType: 'foo',
      });
    });

    test('should create a load request action with filter', () => {
      const filter = Filter.fromString('type=abc');
      const actions = createEntitiesActions('foo');
      const action = actions.request(filter);

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_REQUEST,
        entityType: 'foo',
        filter,
      });
    });

    test('should create a load success action', () => {
      const actions = createEntitiesActions('foo');
      const action = actions.success(['foo', 'bar']);

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_SUCCESS,
        entityType: 'foo',
        data: ['foo', 'bar'],
      });
    });

    test('should create a load success action with filter', () => {
      const filter = Filter.fromString('type=abc');
      const actions = createEntitiesActions('foo');
      const action = actions.success(['foo', 'bar'], filter);

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_SUCCESS,
        entityType: 'foo',
        data: ['foo', 'bar'],
        filter,
      });
    });

    test('should create a load error action', () => {
      const actions = createEntitiesActions('foo');
      const action = actions.error('An error');

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_ERROR,
        entityType: 'foo',
        error: 'An error',
      });
    });

    test('should create a load error action with filter', () => {
      const filter = Filter.fromString('type=abc');
      const actions = createEntitiesActions('foo');
      const action = actions.error('An error', filter);

      expect(action).toEqual({
        type: types.ENTITIES_LOADING_ERROR,
        entityType: 'foo',
        error: 'An error',
        filter,
      });
    });
  });

  describe('createEntityActions tests', () => {

    test('should create actions for loading', () => {
      const actions = createEntityActions('foo');

      expect(actions.request).toBeDefined();
      expect(is_function(actions.request)).toBe(true);
      expect(actions.success).toBeDefined();
      expect(is_function(actions.success)).toBe(true);
      expect(actions.error).toBeDefined();
      expect(is_function(actions.error)).toBe(true);
    });

    test('should create a load request action', () => {
      const actions = createEntityActions('foo');
      const action = actions.request('id1');

      expect(action).toEqual({
        type: types.ENTITY_LOADING_REQUEST,
        entityType: 'foo',
        id: 'id1',
      });
    });

    test('should create a load success action', () => {
      const actions = createEntityActions('foo');
      const action = actions.success('id1', {foo: 'bar'});

      expect(action).toEqual({
        type: types.ENTITY_LOADING_SUCCESS,
        entityType: 'foo',
        id: 'id1',
        data: {foo: 'bar'},
      });
    });

    test('should create a load error action', () => {
      const actions = createEntityActions('foo');
      const action = actions.error('id1', 'An error');

      expect(action).toEqual({
        type: types.ENTITY_LOADING_ERROR,
        entityType: 'foo',
        id: 'id1',
        error: 'An error',
      });
    });
  });

  describe('createLoadEntities tests', () => {

    test('test isLoading true', () => {
      const actions = createEntitiesActions('foo');

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const isLoadingEntities = jest
        .fn()
        .mockReturnValue(true);
      const getAll = jest.fn();
      const gmp = {
        foos: {
          getAll,
        },
      };

      const selector = jest.fn(() => ({
        isLoadingEntities,
      }));

      const loadEntities = createLoadEntities({
        selector,
        actions,
        entityType: 'foo',
      });

      expect(loadEntities).toBeDefined();
      expect(is_function(loadEntities)).toBe(true);

      return loadEntities({gmp})(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntities).toBeCalled();
        expect(dispatch).not.toBeCalled();
        expect(getAll).not.toBeCalled();
      });
    });

    test('test loading success', () => {
      const actions = {
        request: jest.fn().mockReturnValue({type: 'MY_REQUEST_ACTION'}),
        success: jest.fn().mockReturnValue({type: 'MY_SUCCESS_ACTION'}),
        error: jest.fn(),
      };

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const getAll = jest
        .fn()
        .mockReturnValue(Promise.resolve({
          data: 'foo',
        }));
      const gmp = {
        foos: {
          getAll,
        },
      };
      const isLoadingEntities = jest
        .fn()
        .mockReturnValue(false);

      const selector = jest.fn(() => ({
        isLoadingEntities,
      }));

      const loadEntities = createLoadEntities({
        selector,
        actions,
        entityType: 'foo',
      });

      const props = {
        gmp,
        filter: 'myfilter',
        other: 3,
      };

      expect(loadEntities).toBeDefined();
      expect(is_function(loadEntities)).toBe(true);

      return loadEntities(props)(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntities).toBeCalledWith('myfilter');
        expect(getAll).toBeCalledWith({filter: 'myfilter'});
        expect(actions.request).toBeCalledWith('myfilter');
        expect(actions.success).toBeCalledWith('foo', 'myfilter');
        expect(actions.error).not.toBeCalled();
        expect(dispatch).toHaveBeenCalledTimes(2);
        expect(dispatch.mock.calls[0]).toEqual([{type: 'MY_REQUEST_ACTION'}]);
        expect(dispatch.mock.calls[1]).toEqual([{type: 'MY_SUCCESS_ACTION'}]);
      });
    });

    test('test loading error', () => {
      const actions = {
        request: jest.fn().mockReturnValue({type: 'MY_REQUEST_ACTION'}),
        success: jest.fn().mockReturnValue({type: 'MY_SUCCESS_ACTION'}),
        error: jest.fn().mockReturnValue({type: 'MY_ERROR_ACTION'}),
      };

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const getAll = jest
        .fn()
        .mockReturnValue(Promise.reject('AnError'));
      const gmp = {
        foos: {
          getAll,
        },
      };
      const isLoadingEntities = jest
        .fn()
        .mockReturnValue(false);

      const selector = jest.fn(() => ({
        isLoadingEntities,
      }));

      const loadEntities = createLoadEntities({
        selector,
        actions,
        entityType: 'foo',
      });

      const props = {
        gmp,
        filter: 'myfilter',
        other: 3,
      };

      expect(loadEntities).toBeDefined();
      expect(is_function(loadEntities)).toBe(true);

      return loadEntities(props)(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntities).toBeCalledWith('myfilter');
        expect(actions.request).toBeCalledWith('myfilter');
        expect(actions.success).not.toBeCalled();
        expect(actions.error).toBeCalledWith('AnError', 'myfilter');
        expect(getAll).toBeCalledWith({filter: 'myfilter'});
        expect(dispatch).toHaveBeenCalledTimes(2);
        expect(dispatch.mock.calls[0]).toEqual([{type: 'MY_REQUEST_ACTION'}]);
        expect(dispatch.mock.calls[1]).toEqual([{type: 'MY_ERROR_ACTION'}]);
      });
    });
  });

  describe('createLoadEntity tests', () => {

    test('test isLoading true', () => {
      const id = 'a1';
      const actions = {
        request: jest.fn().mockReturnValue({type: 'MY_REQUEST_ACTION'}),
        success: jest.fn().mockReturnValue({type: 'MY_SUCCESS_ACTION'}),
        error: jest.fn().mockReturnValue({type: 'MY_ERROR_ACTION'}),
      };

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const isLoadingEntity = jest
        .fn()
        .mockReturnValue(true);
      const get = jest
        .fn()
        .mockReturnValue(Promise.resolve({id: 'foo'}));
      const gmp = {
        foo: {
          get,
        },
      };

      const selector = jest.fn(() => ({
        isLoadingEntity,
      }));

      const loadEntity = createLoadEntity({
        selector,
        actions,
        entityType: 'foo',
      });

      expect(loadEntity).toBeDefined();
      expect(is_function(loadEntity)).toBe(true);

      return loadEntity({gmp, id})(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntity).toBeCalledWith(id);
        expect(dispatch).not.toBeCalled();
        expect(actions.success).not.toBeCalled();
        expect(actions.error).not.toBeCalled();
        expect(actions.request).not.toBeCalled();
        expect(get).not.toBeCalled();
      });
    });

    test('test loading success', () => {
      const id = 'a1';
      const actions = {
        request: jest.fn().mockReturnValue({type: 'MY_REQUEST_ACTION'}),
        success: jest.fn().mockReturnValue({type: 'MY_SUCCESS_ACTION'}),
        error: jest.fn().mockReturnValue({type: 'MY_ERROR_ACTION'}),
      };

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const isLoadingEntity = jest
        .fn()
        .mockReturnValue(false);
      const get = jest
        .fn()
        .mockReturnValue(Promise.resolve({data: {id, name: 'foo'}}));
      const gmp = {
        foo: {
          get,
        },
      };

      const selector = jest.fn(() => ({
        isLoadingEntity,
      }));

      const loadEntity = createLoadEntity({
        selector,
        actions,
        entityType: 'foo',
      });

      expect(loadEntity).toBeDefined();
      expect(is_function(loadEntity)).toBe(true);

      return loadEntity({gmp, id})(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntity).toBeCalledWith(id);
        expect(actions.request).toBeCalledWith(id);
        expect(actions.success).toBeCalledWith(id, {id, name: 'foo'});
        expect(actions.error).not.toBeCalled();
        expect(dispatch).toHaveBeenCalledTimes(2);
        expect(dispatch.mock.calls[0]).toEqual([{type: 'MY_REQUEST_ACTION'}]);
        expect(dispatch.mock.calls[1]).toEqual([{type: 'MY_SUCCESS_ACTION'}]);
        expect(get).toBeCalledWith({id});
      });
    });

    test('test loading error', () => {
      const id = 'a1';
      const actions = {
        request: jest.fn().mockReturnValue({type: 'MY_REQUEST_ACTION'}),
        success: jest.fn().mockReturnValue({type: 'MY_SUCCESS_ACTION'}),
        error: jest.fn().mockReturnValue({type: 'MY_ERROR_ACTION'}),
      };

      const getState = jest
        .fn()
        .mockReturnValue({foo: 'bar'});

      const dispatch = jest.fn();
      const isLoadingEntity = jest
        .fn()
        .mockReturnValue(false);
      const get = jest
        .fn()
        .mockReturnValue(Promise.reject('An error'));
      const gmp = {
        foo: {
          get,
        },
      };

      const selector = jest.fn(() => ({
        isLoadingEntity,
      }));

      const loadEntity = createLoadEntity({
        selector,
        actions,
        entityType: 'foo',
      });

      expect(loadEntity).toBeDefined();
      expect(is_function(loadEntity)).toBe(true);

      return loadEntity({gmp, id})(dispatch, getState).then(() => {
        expect(getState).toBeCalled();
        expect(selector).toBeCalledWith({foo: 'bar'});
        expect(isLoadingEntity).toBeCalledWith(id);
        expect(actions.request).toBeCalledWith(id);
        expect(actions.success).not.toBeCalled();
        expect(actions.error).toBeCalledWith(id, 'An error');
        expect(dispatch).toHaveBeenCalledTimes(2);
        expect(dispatch.mock.calls[0]).toEqual([{type: 'MY_REQUEST_ACTION'}]);
        expect(dispatch.mock.calls[1]).toEqual([{type: 'MY_ERROR_ACTION'}]);
        expect(get).toBeCalledWith({id});
      });
    });
  });

});
// vim: set ts=2 sw=2 tw=80:
